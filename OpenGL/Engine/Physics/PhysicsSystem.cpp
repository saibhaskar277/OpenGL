#include "Engine/Physics/PhysicsSystem.h"

#include "Engine/Physics/ColliderFactory.h"
#include "Engine/Physics/PhysicsComponents.h"
#include "Engine/Scene/Components.h"

#include <iostream>
#include <unordered_set>

namespace Engine {
namespace {

bool entityWantsPhysicsBody(const Registry& registry, Entity entity) {
    if (!registry.has<ColliderComponent>(entity) ||
        !registry.has<TransformComponent>(entity)) {
        return false;
    }
    if (!registry.has<RigidBodyComponent>(entity)) {
        // Collider-only = static geometry (floor, walls, etc.).
        return true;
    }
    return registry.get<RigidBodyComponent>(entity).enabled;
}

PhysicsMotionType motionForEntity(const Registry& registry, Entity entity) {
    if (registry.has<RigidBodyComponent>(entity)) {
        return registry.get<RigidBodyComponent>(entity).motionType;
    }
    return PhysicsMotionType::Static;
}

bool bodyNeedsRebuild(Registry& registry, Entity entity, PhysicsBodyId existingId) {
    if (existingId == InvalidPhysicsBodyId) {
        return true;
    }
    if (registry.has<RigidBodyComponent>(entity)) {
        return registry.get<RigidBodyComponent>(entity).needsRebuild;
    }
    return registry.get<ColliderComponent>(entity).needsRebuild;
}

void clearRebuildFlags(Registry& registry, Entity entity) {
    if (registry.has<RigidBodyComponent>(entity)) {
        registry.get<RigidBodyComponent>(entity).needsRebuild = false;
    }
    if (registry.has<ColliderComponent>(entity)) {
        registry.get<ColliderComponent>(entity).needsRebuild = false;
    }
}

} // namespace

PhysicsSystem::PhysicsSystem(IPhysicsWorld& world)
    : m_world(world) {
}

void PhysicsSystem::setSimulationEnabled(bool enabled) {
    m_simulationEnabled = enabled;
}

bool PhysicsSystem::simulationEnabled() const {
    return m_simulationEnabled;
}

void PhysicsSystem::rebuild(Scene& scene) {
    m_world.clear();
    m_entityBodies.clear();
    for (auto& [entity, body] : scene.registry().storage<RigidBodyComponent>()) {
        (void)entity;
        body.bodyId = InvalidPhysicsBodyId;
        body.needsRebuild = true;
    }
    for (auto& [entity, collider] : scene.registry().storage<ColliderComponent>()) {
        (void)entity;
        collider.needsRebuild = true;
    }
    ensureBodies(scene);
}

void PhysicsSystem::ensureBodies(Scene& scene) {
    Registry& registry = scene.registry();

    std::unordered_set<Entity> live;
    for (Entity entity : registry.entities()) {
        if (entityWantsPhysicsBody(registry, entity)) {
            live.insert(entity);
        }
    }

    for (auto it = m_entityBodies.begin(); it != m_entityBodies.end();) {
        if (live.find(it->first) == live.end()) {
            m_world.destroyBody(it->second);
            if (registry.has<RigidBodyComponent>(it->first)) {
                registry.get<RigidBodyComponent>(it->first).bodyId = InvalidPhysicsBodyId;
            }
            it = m_entityBodies.erase(it);
        } else {
            ++it;
        }
    }

    for (Entity entity : live) {
        const auto existing = m_entityBodies.find(entity);
        const PhysicsBodyId existingId =
            existing != m_entityBodies.end() ? existing->second : InvalidPhysicsBodyId;

        if (!bodyNeedsRebuild(registry, entity, existingId)) {
            continue;
        }

        if (existing != m_entityBodies.end()) {
            m_world.destroyBody(existing->second);
            m_entityBodies.erase(existing);
        }

        const Transform& transform =
            registry.get<TransformComponent>(entity).transform;
        const ColliderComponent& collider =
            registry.get<ColliderComponent>(entity);

        PhysicsBodyDesc desc;
        desc.motion = motionForEntity(registry, entity);
        desc.position = transform.position();
        desc.rotationDegrees = transform.rotationDegrees();
        desc.collider = ColliderFactory::toShapeDesc(collider);
        desc.userData = static_cast<std::uint64_t>(entity);

        if (registry.has<RigidBodyComponent>(entity)) {
            const RigidBodyComponent& body = registry.get<RigidBodyComponent>(entity);
            desc.mass = body.mass;
            desc.friction = body.friction;
            desc.restitution = body.restitution;
            desc.enableGravity = body.enableGravity;
        } else {
            desc.mass = 0.0f;
            desc.friction = 0.5f;
            desc.restitution = 0.0f;
            desc.enableGravity = false;
        }

        try {
            const PhysicsBodyId bodyId = m_world.createBody(desc);
            m_entityBodies[entity] = bodyId;
            if (registry.has<RigidBodyComponent>(entity)) {
                registry.get<RigidBodyComponent>(entity).bodyId = bodyId;
            }
            clearRebuildFlags(registry, entity);
        } catch (const std::exception& error) {
            std::cerr << "[Physics] Failed to create body for entity "
                      << entity << ": " << error.what() << '\n';
            if (registry.has<RigidBodyComponent>(entity)) {
                registry.get<RigidBodyComponent>(entity).bodyId = InvalidPhysicsBodyId;
            }
            clearRebuildFlags(registry, entity);
        }
    }
}

void PhysicsSystem::syncTransformsToBodies(Scene& scene) {
    Registry& registry = scene.registry();
    for (const auto& [entity, bodyId] : m_entityBodies) {
        if (!registry.has<TransformComponent>(entity)) {
            continue;
        }

        const bool isDynamic =
            registry.has<RigidBodyComponent>(entity) &&
            registry.get<RigidBodyComponent>(entity).enabled &&
            registry.get<RigidBodyComponent>(entity).motionType ==
                PhysicsMotionType::Dynamic;

        if (!m_simulationEnabled || !isDynamic) {
            const Transform& transform =
                registry.get<TransformComponent>(entity).transform;
            m_world.setBodyTransform(
                bodyId,
                transform.position(),
                transform.rotationDegrees());
        }
    }
}

void PhysicsSystem::syncBodiesToTransforms(Scene& scene) {
    if (!m_simulationEnabled) {
        return;
    }

    Registry& registry = scene.registry();
    for (auto& [entity, body] : registry.storage<RigidBodyComponent>()) {
        if (!body.enabled || body.bodyId == InvalidPhysicsBodyId) {
            continue;
        }
        if (body.motionType != PhysicsMotionType::Dynamic) {
            continue;
        }
        if (!registry.has<TransformComponent>(entity)) {
            continue;
        }

        glm::vec3 position;
        glm::vec3 rotationDegrees;
        m_world.getBodyTransform(body.bodyId, position, rotationDegrees);
        Transform& transform = registry.get<TransformComponent>(entity).transform;
        transform.setPosition(position);
        transform.setRotationDegrees(rotationDegrees);
    }
}

void PhysicsSystem::update(
    Scene& scene,
    float deltaTime,
    bool transformsAreAuthoritative) {
    ensureBodies(scene);

    if (transformsAreAuthoritative || !m_simulationEnabled) {
        syncTransformsToBodies(scene);
    }

    if (m_simulationEnabled) {
        m_world.step(deltaTime);
        syncBodiesToTransforms(scene);
    }
}

} // namespace Engine
