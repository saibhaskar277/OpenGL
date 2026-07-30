#include "Engine/Scene/Scene.h"

#include "Engine/Physics/ColliderFactory.h"
#include "Engine/Physics/PhysicsComponents.h"
#include "Engine/Scene/Components.h"

#include <glm/glm.hpp>

namespace Engine {

Registry& Scene::registry() {
    return m_registry;
}

const Registry& Scene::registry() const {
    return m_registry;
}

Entity Scene::createEntity(const std::string& name) {
    const Entity entity = m_registry.createEntity();
    m_registry.add<TagComponent>(entity, TagComponent{name});
    m_registry.add<TransformComponent>(entity);
    return entity;
}

void Scene::destroyEntity(Entity entity) {
    if (m_selectedEntity == entity) {
        m_selectedEntity = NullEntity;
    }
    m_registry.destroyEntity(entity);
}

void Scene::clear() {
    m_selectedEntity = NullEntity;
    m_registry.clear();
}

Entity Scene::createCube(const std::string& name) {
    const Entity entity = createEntity(name);
    m_registry.add<MeshRendererComponent>(entity);
    m_registry.add<MaterialComponent>(entity);
    return entity;
}

Entity Scene::createPhysicsCube(const std::string& name, PhysicsMotionType motion) {
    const Entity entity = createCube(name);
    auto& transform = m_registry.get<TransformComponent>(entity).transform;
    transform.setPosition(glm::vec3(0.0f, 2.0f, 0.0f));

    RigidBodyComponent body;
    body.motionType = motion;
    body.mass = 1.0f;
    body.needsRebuild = true;
    m_registry.add<RigidBodyComponent>(entity, body);
    m_registry.add<ColliderComponent>(
        entity,
        ColliderFactory::boxFromScale(transform.scale()));
    return entity;
}

Entity Scene::createModel(const std::string& modelPath, const std::string& name) {
    const Entity entity = createEntity(name);
    MeshRendererComponent mesh;
    mesh.meshType = MeshType::Model;
    mesh.modelPath = modelPath;
    m_registry.add<MeshRendererComponent>(entity, mesh);
    m_registry.add<MaterialComponent>(entity);
    return entity;
}

Entity Scene::createDirectionalLight(const std::string& name) {
    const Entity entity = createEntity(name);
    m_registry.get<TransformComponent>(entity).transform.setPosition(
        glm::vec3(0.0f, 4.0f, 0.0f));

    DirectionalLightComponent component;
    component.light.direction = glm::vec3(-0.4f, -1.0f, -0.25f);
    component.light.color = glm::vec3(0.72f, 0.82f, 1.0f);
    component.light.intensity = 0.75f;
    m_registry.add<DirectionalLightComponent>(entity, component);
    return entity;
}

Entity Scene::createPointLight(const std::string& name) {
    const Entity entity = createEntity(name);
    const glm::vec3 lightPosition(2.0f, 1.8f, 1.5f);
    m_registry.get<TransformComponent>(entity).transform.setPosition(lightPosition);

    PointLightComponent component;
    component.light.position = lightPosition;
    component.light.color = glm::vec3(1.0f, 0.75f, 0.45f);
    component.light.intensity = 3.0f;
    m_registry.add<PointLightComponent>(entity, component);
    return entity;
}

Entity Scene::createSpotLight(const std::string& name) {
    const Entity entity = createEntity(name);
    const glm::vec3 lightPosition(0.0f, 2.5f, 2.0f);
    m_registry.get<TransformComponent>(entity).transform.setPosition(lightPosition);

    SpotLightComponent component;
    component.light.position = lightPosition;
    component.light.direction = glm::vec3(0.0f, -1.0f, -0.35f);
    component.light.color = glm::vec3(0.55f, 0.72f, 1.0f);
    component.light.intensity = 2.0f;
    component.followCamera = false;
    m_registry.add<SpotLightComponent>(entity, component);
    return entity;
}

Entity Scene::createHemisphereLight(const std::string& name) {
    const Entity entity = createEntity(name);

    HemisphereLightComponent component;
    component.light.skyColor = glm::vec3(0.40f, 0.55f, 0.85f);
    component.light.groundColor = glm::vec3(0.18f, 0.14f, 0.10f);
    component.light.intensity = 0.35f;
    component.light.upDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    component.light.enabled = true;
    m_registry.add<HemisphereLightComponent>(entity, component);
    return entity;
}

void Scene::createDefaultScene() {
    clear();

    Entity floor = createCube("Floor");
    {
        auto& transform = m_registry.get<TransformComponent>(floor).transform;
        transform.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
        transform.setScale(glm::vec3(8.0f, 0.1f, 8.0f));
        auto& material = m_registry.get<MaterialComponent>(floor);
        material.albedo = glm::vec3(0.75f);
        material.specularColor = glm::vec3(0.2f);
        material.shininess = 24.0f;

        RigidBodyComponent body;
        body.motionType = PhysicsMotionType::Static;
        body.needsRebuild = true;
        m_registry.add<RigidBodyComponent>(floor, body);
        m_registry.add<ColliderComponent>(
            floor,
            ColliderFactory::boxFromScale(transform.scale()));
    }

    Entity cube = createPhysicsCube("Physics Cube", PhysicsMotionType::Dynamic);
    {
        auto& transform = m_registry.get<TransformComponent>(cube).transform;
        transform.setPosition(glm::vec3(0.0f, 2.0f, 0.0f));
        transform.setRotationDegrees(glm::vec3(-15.0f, 30.0f, 0.0f));
        auto& material = m_registry.get<MaterialComponent>(cube);
        material.albedo = glm::vec3(1.0f, 0.55f, 0.2f);
        material.shininess = 64.0f;
        m_registry.get<ColliderComponent>(cube) =
            ColliderFactory::boxFromScale(transform.scale());
        m_registry.get<RigidBodyComponent>(cube).needsRebuild = true;
    }

    {
        Entity sun = createDirectionalLight("Directional Light");
        auto& light = m_registry.get<DirectionalLightComponent>(sun).light;
        light.color = glm::vec3(0.72f, 0.82f, 1.0f);
        light.intensity = 0.75f;
    }

    {
        Entity point = createPointLight("Point Light");
        auto& light = m_registry.get<PointLightComponent>(point).light;
        light.color = glm::vec3(1.0f, 0.32f, 0.08f);
        light.intensity = 5.0f;
    }

    {
        Entity spot = createSpotLight("Spot Light");
        auto& component = m_registry.get<SpotLightComponent>(spot);
        component.followCamera = true;
        component.light.intensity = 2.0f;
    }

    createHemisphereLight("Hemisphere Light");

    setSelectedEntity(cube);
}

void Scene::buildLightingEnvironment(LightingEnvironment& lighting) const {
    lighting.clearPointLights();

    lighting.directionalLight().enabled = false;
    lighting.spotLight().enabled = false;
    lighting.hemisphereLight() = HemisphereLight{};
    lighting.hemisphereLight().enabled = false;

    for (const auto& [entity, component] : m_registry.storage<DirectionalLightComponent>()) {
        (void)entity;
        lighting.directionalLight() = component.light;
        break;
    }

    for (const auto& [entity, component] : m_registry.storage<PointLightComponent>()) {
        (void)entity;
        if (lighting.pointLights().size() >= LightingEnvironment::MaxPointLights) {
            break;
        }
        lighting.addPointLight(component.light);
    }

    for (const auto& [entity, component] : m_registry.storage<SpotLightComponent>()) {
        (void)entity;
        lighting.spotLight() = component.light;
        break;
    }

    for (const auto& [entity, component] : m_registry.storage<HemisphereLightComponent>()) {
        (void)entity;
        lighting.hemisphereLight() = component.light;
        break;
    }
}

Entity Scene::selectedEntity() const {
    return m_selectedEntity;
}

void Scene::setSelectedEntity(Entity entity) {
    m_selectedEntity = entity;
}

} // namespace Engine
