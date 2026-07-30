#include "Engine/Physics/Jolt/JoltPhysicsWorld.h"

#include "Engine/Physics/PhysicsMath.h"

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/RegisterTypes.h>

#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace Engine {
namespace {

void joltTrace(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
#if defined(_MSC_VER)
    vsnprintf_s(buffer, sizeof(buffer), format, args);
#else
    vsnprintf(buffer, sizeof(buffer), format, args);
#endif
    va_end(args);
    std::cerr << "[Jolt] " << buffer << '\n';
}

JPH::EMotionType toJoltMotion(PhysicsMotionType motion) {
    switch (motion) {
    case PhysicsMotionType::Static:
        return JPH::EMotionType::Static;
    case PhysicsMotionType::Kinematic:
        return JPH::EMotionType::Kinematic;
    case PhysicsMotionType::Dynamic:
    default:
        return JPH::EMotionType::Dynamic;
    }
}

JPH::ObjectLayer layerForMotion(PhysicsMotionType motion) {
    return motion == PhysicsMotionType::Static ? 0 : 1;
}

JPH::Vec3 toJolt(const glm::vec3& value) {
    return JPH::Vec3(value.x, value.y, value.z);
}

glm::vec3 toGlm(const JPH::Vec3& value) {
    return glm::vec3(value.GetX(), value.GetY(), value.GetZ());
}

JPH::Quat toJoltQuat(const glm::vec3& rotationDegrees) {
    const glm::quat q = PhysicsMath::rotationDegreesToQuat(rotationDegrees);
    return JPH::Quat(q.x, q.y, q.z, q.w);
}

glm::vec3 toGlmDegrees(const JPH::Quat& rotation) {
    const glm::quat q(rotation.GetW(), rotation.GetX(), rotation.GetY(), rotation.GetZ());
    return PhysicsMath::quatToRotationDegrees(q);
}

JPH::RefConst<JPH::Shape> createShape(const ColliderShapeDesc& collider) {
    JPH::ShapeSettings::ShapeResult result;
    switch (collider.type) {
    case ColliderShapeType::Sphere: {
        JPH::SphereShapeSettings settings(std::max(collider.sphereRadius, 0.05f));
        result = settings.Create();
        break;
    }
    case ColliderShapeType::Capsule: {
        JPH::CapsuleShapeSettings settings(
            std::max(collider.capsuleHalfHeight, 0.05f),
            std::max(collider.capsuleRadius, 0.05f));
        result = settings.Create();
        break;
    }
    case ColliderShapeType::Box:
    default: {
        // Jolt requires halfExtent > convexRadius (strict). Thin floors
        // (scale.y = 0.1 → half = 0.05) fail with the default 0.05 radius.
        const JPH::Vec3 half(
            std::max(collider.boxHalfExtents.x, 0.01f),
            std::max(collider.boxHalfExtents.y, 0.01f),
            std::max(collider.boxHalfExtents.z, 0.01f));
        const float minHalf = half.ReduceMin();
        const float convexRadius = std::min(0.05f, minHalf * 0.5f);
        JPH::BoxShapeSettings settings(half, convexRadius);
        result = settings.Create();
        break;
    }
    }

    if (result.HasError()) {
        throw std::runtime_error(result.GetError().c_str());
    }

    JPH::RefConst<JPH::Shape> shape = result.Get();
    if (glm::length(collider.localOffset) > 0.0001f) {
        JPH::RotatedTranslatedShapeSettings offsetSettings(
            toJolt(collider.localOffset),
            JPH::Quat::sIdentity(),
            shape);
        auto offsetResult = offsetSettings.Create();
        if (offsetResult.HasError()) {
            throw std::runtime_error(offsetResult.GetError().c_str());
        }
        shape = offsetResult.Get();
    }
    return shape;
}

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
public:
    BroadPhaseLayerInterfaceImpl() {
        m_objectToBroadPhase[0] = JPH::BroadPhaseLayer(0);
        m_objectToBroadPhase[1] = JPH::BroadPhaseLayer(1);
    }

    unsigned int GetNumBroadPhaseLayers() const override {
        return 2;
    }

    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
        return m_objectToBroadPhase[layer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override {
        switch ((JPH::BroadPhaseLayer::Type)layer) {
        case 0:
            return "Static";
        case 1:
            return "Dynamic";
        default:
            return "Invalid";
        }
    }
#endif

private:
    JPH::BroadPhaseLayer m_objectToBroadPhase[2];
};

class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override {
        (void)layer1;
        (void)layer2;
        return true;
    }
};

class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
public:
    bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override {
        (void)a;
        (void)b;
        return true;
    }
};

BroadPhaseLayerInterfaceImpl g_broadPhaseLayers;
ObjectVsBroadPhaseLayerFilterImpl g_objectVsBroadPhase;
ObjectLayerPairFilterImpl g_objectLayerPair;
bool g_joltInitialized = false;

void ensureJoltInitialized() {
    if (g_joltInitialized) {
        return;
    }
    JPH::RegisterDefaultAllocator();
    JPH::Trace = joltTrace;
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    g_joltInitialized = true;
}

} // namespace

JoltPhysicsWorld::JoltPhysicsWorld() {
    ensureJoltInitialized();

    m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(16 * 1024 * 1024);
    const int workerThreads = std::max(
        1,
        static_cast<int>(std::thread::hardware_concurrency()) - 1);
    m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
        JPH::cMaxPhysicsJobs,
        JPH::cMaxPhysicsBarriers,
        workerThreads);

    m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();
    m_physicsSystem->Init(
        2048,
        0,
        2048,
        2048,
        g_broadPhaseLayers,
        g_objectVsBroadPhase,
        g_objectLayerPair);
    m_physicsSystem->SetGravity(toJolt(m_gravity));
}

JoltPhysicsWorld::~JoltPhysicsWorld() {
    clear();
    m_physicsSystem.reset();
    m_jobSystem.reset();
    m_tempAllocator.reset();
}

void JoltPhysicsWorld::setGravity(const glm::vec3& gravity) {
    m_gravity = gravity;
    if (m_physicsSystem) {
        m_physicsSystem->SetGravity(toJolt(gravity));
    }
}

glm::vec3 JoltPhysicsWorld::gravity() const {
    return m_gravity;
}

bool JoltPhysicsWorld::isValid(PhysicsBodyId bodyId) const {
    return bodyId != InvalidPhysicsBodyId && m_bodies.find(bodyId) != m_bodies.end();
}

JPH::BodyID JoltPhysicsWorld::toJoltId(PhysicsBodyId bodyId) const {
    return m_bodies.at(bodyId);
}

PhysicsBodyId JoltPhysicsWorld::createBody(const PhysicsBodyDesc& desc) {
    JPH::RefConst<JPH::Shape> shape = createShape(desc.collider);

    JPH::BodyCreationSettings settings(
        shape,
        JPH::RVec3(toJolt(desc.position)),
        toJoltQuat(desc.rotationDegrees),
        toJoltMotion(desc.motion),
        layerForMotion(desc.motion));
    settings.mFriction = desc.friction;
    settings.mRestitution = desc.restitution;
    settings.mGravityFactor = desc.enableGravity ? 1.0f : 0.0f;
    settings.mUserData = desc.userData;

    if (desc.motion == PhysicsMotionType::Dynamic) {
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = std::max(desc.mass, 0.01f);
    }

    JPH::BodyInterface& bodies = m_physicsSystem->GetBodyInterface();
    const JPH::BodyID joltId = bodies.CreateAndAddBody(settings, JPH::EActivation::Activate);

    const PhysicsBodyId id = m_nextBodyId++;
    m_bodies.emplace(id, joltId);
    return id;
}

void JoltPhysicsWorld::destroyBody(PhysicsBodyId bodyId) {
    if (!isValid(bodyId)) {
        return;
    }
    JPH::BodyInterface& bodies = m_physicsSystem->GetBodyInterface();
    const JPH::BodyID joltId = toJoltId(bodyId);
    bodies.RemoveBody(joltId);
    bodies.DestroyBody(joltId);
    m_bodies.erase(bodyId);
}

void JoltPhysicsWorld::clear() {
    JPH::BodyInterface& bodies = m_physicsSystem->GetBodyInterface();
    for (const auto& [id, joltId] : m_bodies) {
        (void)id;
        bodies.RemoveBody(joltId);
        bodies.DestroyBody(joltId);
    }
    m_bodies.clear();
    m_accumulator = 0.0f;
}

void JoltPhysicsWorld::setBodyTransform(
    PhysicsBodyId bodyId,
    const glm::vec3& position,
    const glm::vec3& rotationDegrees) {
    if (!isValid(bodyId)) {
        return;
    }
    JPH::BodyInterface& bodies = m_physicsSystem->GetBodyInterface();
    bodies.SetPositionAndRotation(
        toJoltId(bodyId),
        JPH::RVec3(toJolt(position)),
        toJoltQuat(rotationDegrees),
        JPH::EActivation::Activate);
}

void JoltPhysicsWorld::getBodyTransform(
    PhysicsBodyId bodyId,
    glm::vec3& outPosition,
    glm::vec3& outRotationDegrees) const {
    if (!isValid(bodyId)) {
        return;
    }
    JPH::RVec3 position;
    JPH::Quat rotation;
    m_physicsSystem->GetBodyInterface().GetPositionAndRotation(
        toJoltId(bodyId),
        position,
        rotation);
    outPosition = glm::vec3(
        static_cast<float>(position.GetX()),
        static_cast<float>(position.GetY()),
        static_cast<float>(position.GetZ()));
    outRotationDegrees = toGlmDegrees(rotation);
}

void JoltPhysicsWorld::setBodyMotionType(PhysicsBodyId bodyId, PhysicsMotionType motion) {
    if (!isValid(bodyId)) {
        return;
    }
    JPH::BodyInterface& bodies = m_physicsSystem->GetBodyInterface();
    const JPH::BodyID joltId = toJoltId(bodyId);
    bodies.SetMotionType(joltId, toJoltMotion(motion), JPH::EActivation::Activate);
    bodies.SetObjectLayer(joltId, layerForMotion(motion));
}

void JoltPhysicsWorld::addImpulse(PhysicsBodyId bodyId, const glm::vec3& impulse) {
    if (!isValid(bodyId)) {
        return;
    }
    m_physicsSystem->GetBodyInterface().AddImpulse(toJoltId(bodyId), toJolt(impulse));
}

void JoltPhysicsWorld::step(float deltaTime) {
    m_accumulator += std::min(deltaTime, 0.1f);
    constexpr int maxSteps = 5;
    int steps = 0;
    while (m_accumulator >= FixedDelta && steps < maxSteps) {
        m_physicsSystem->Update(
            FixedDelta,
            1,
            m_tempAllocator.get(),
            m_jobSystem.get());
        m_accumulator -= FixedDelta;
        ++steps;
    }
}

std::unique_ptr<IPhysicsWorld> createJoltPhysicsWorld() {
    return std::make_unique<JoltPhysicsWorld>();
}

} // namespace Engine
