#include "Engine/Scene/SceneSerializer.h"

#include "Engine/Scene/Components.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

namespace Engine {
namespace {

using json = nlohmann::json;

json vec3ToJson(const glm::vec3& value) {
    return json::array({value.x, value.y, value.z});
}

glm::vec3 vec3FromJson(const json& value) {
    return glm::vec3(
        value.at(0).get<float>(),
        value.at(1).get<float>(),
        value.at(2).get<float>());
}

} // namespace

SceneSerializer::SceneSerializer(Scene& scene)
    : m_scene(scene) {
}

void SceneSerializer::save(const std::filesystem::path& path) const {
    json root;
    root["version"] = 1;
    json entities = json::array();

    const Registry& registry = m_scene.registry();
    for (Entity entity : registry.entities()) {
        json entityJson;
        entityJson["id"] = entity;

        if (registry.has<TagComponent>(entity)) {
            entityJson["tag"]["name"] = registry.get<TagComponent>(entity).name;
        }
        if (registry.has<TransformComponent>(entity)) {
            const Transform& transform = registry.get<TransformComponent>(entity).transform;
            entityJson["transform"] = {
                {"position", vec3ToJson(transform.position())},
                {"rotation", vec3ToJson(transform.rotationDegrees())},
                {"scale", vec3ToJson(transform.scale())}};
        }
        if (registry.has<MeshRendererComponent>(entity)) {
            const auto& mesh = registry.get<MeshRendererComponent>(entity);
            entityJson["meshRenderer"] = {
                {"meshType", mesh.meshType == MeshType::Model ? "Model" : "Cube"},
                {"modelPath", mesh.modelPath},
                {"castShadows", mesh.castShadows},
                {"visible", mesh.visible}};
        }
        if (registry.has<MaterialComponent>(entity)) {
            const auto& material = registry.get<MaterialComponent>(entity);
            entityJson["material"] = {
                {"albedo", vec3ToJson(material.albedo)},
                {"specularColor", vec3ToJson(material.specularColor)},
                {"shininess", material.shininess},
                {"emissive", vec3ToJson(material.emissive)},
                {"ambientOcclusion", material.ambientOcclusion},
                {"albedoMapPath", material.albedoMapPath},
                {"normalMapPath", material.normalMapPath},
                {"specularMapPath", material.specularMapPath},
                {"emissiveMapPath", material.emissiveMapPath},
                {"aoMapPath", material.aoMapPath}};
        }
        if (registry.has<DirectionalLightComponent>(entity)) {
            const auto& light = registry.get<DirectionalLightComponent>(entity).light;
            entityJson["directionalLight"] = {
                {"direction", vec3ToJson(light.direction)},
                {"color", vec3ToJson(light.color)},
                {"intensity", light.intensity},
                {"enabled", light.enabled}};
        }
        if (registry.has<PointLightComponent>(entity)) {
            const auto& light = registry.get<PointLightComponent>(entity).light;
            entityJson["pointLight"] = {
                {"position", vec3ToJson(light.position)},
                {"color", vec3ToJson(light.color)},
                {"intensity", light.intensity},
                {"constantAttenuation", light.constantAttenuation},
                {"linearAttenuation", light.linearAttenuation},
                {"quadraticAttenuation", light.quadraticAttenuation}};
        }
        if (registry.has<SpotLightComponent>(entity)) {
            const auto& component = registry.get<SpotLightComponent>(entity);
            const auto& light = component.light;
            entityJson["spotLight"] = {
                {"position", vec3ToJson(light.position)},
                {"direction", vec3ToJson(light.direction)},
                {"color", vec3ToJson(light.color)},
                {"intensity", light.intensity},
                {"innerCutoffDegrees", light.innerCutoffDegrees},
                {"outerCutoffDegrees", light.outerCutoffDegrees},
                {"constantAttenuation", light.constantAttenuation},
                {"linearAttenuation", light.linearAttenuation},
                {"quadraticAttenuation", light.quadraticAttenuation},
                {"enabled", light.enabled},
                {"followCamera", component.followCamera}};
        }
        if (registry.has<HemisphereLightComponent>(entity)) {
            const auto& light = registry.get<HemisphereLightComponent>(entity).light;
            entityJson["hemisphereLight"] = {
                {"skyColor", vec3ToJson(light.skyColor)},
                {"groundColor", vec3ToJson(light.groundColor)},
                {"intensity", light.intensity},
                {"upDirection", vec3ToJson(light.upDirection)},
                {"enabled", light.enabled}};
        }
        if (registry.has<RigidBodyComponent>(entity)) {
            const auto& body = registry.get<RigidBodyComponent>(entity);
            entityJson["rigidBody"] = {
                {"motionType", static_cast<int>(body.motionType)},
                {"mass", body.mass},
                {"friction", body.friction},
                {"restitution", body.restitution},
                {"enableGravity", body.enableGravity},
                {"enabled", body.enabled}};
        }
        if (registry.has<ColliderComponent>(entity)) {
            const auto& collider = registry.get<ColliderComponent>(entity);
            entityJson["collider"] = {
                {"shape", static_cast<int>(collider.shape)},
                {"boxHalfExtents", vec3ToJson(collider.boxHalfExtents)},
                {"sphereRadius", collider.sphereRadius},
                {"capsuleRadius", collider.capsuleRadius},
                {"capsuleHalfHeight", collider.capsuleHalfHeight},
                {"localOffset", vec3ToJson(collider.localOffset)}};
        }

        entities.push_back(std::move(entityJson));
    }

    root["entities"] = std::move(entities);
    root["selectedEntity"] = m_scene.selectedEntity();

    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to write scene: " + path.string());
    }
    file << root.dump(2);
}

void SceneSerializer::load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to read scene: " + path.string());
    }

    json root;
    file >> root;
    m_scene.clear();

    Registry& registry = m_scene.registry();
    for (const json& entityJson : root.at("entities")) {
        const std::string name =
            entityJson.contains("tag")
            ? entityJson.at("tag").at("name").get<std::string>()
            : "Entity";
        const Entity entity = m_scene.createEntity(name);

        if (entityJson.contains("transform")) {
            const json& transformJson = entityJson.at("transform");
            Transform& transform = registry.get<TransformComponent>(entity).transform;
            transform.setPosition(vec3FromJson(transformJson.at("position")));
            transform.setRotationDegrees(vec3FromJson(transformJson.at("rotation")));
            transform.setScale(vec3FromJson(transformJson.at("scale")));
        }

        if (entityJson.contains("meshRenderer")) {
            const json& meshJson = entityJson.at("meshRenderer");
            MeshRendererComponent mesh;
            mesh.meshType =
                meshJson.at("meshType").get<std::string>() == "Model"
                ? MeshType::Model
                : MeshType::Cube;
            mesh.modelPath = meshJson.value("modelPath", "");
            mesh.castShadows = meshJson.value("castShadows", true);
            mesh.visible = meshJson.value("visible", true);
            registry.add<MeshRendererComponent>(entity, mesh);
        }

        if (entityJson.contains("material")) {
            const json& materialJson = entityJson.at("material");
            MaterialComponent material;
            material.albedo = vec3FromJson(materialJson.at("albedo"));
            material.specularColor = vec3FromJson(materialJson.at("specularColor"));
            material.shininess = materialJson.at("shininess").get<float>();
            material.emissive = vec3FromJson(materialJson.value("emissive", json::array({0, 0, 0})));
            material.ambientOcclusion = materialJson.value("ambientOcclusion", 1.0f);
            material.albedoMapPath = materialJson.value("albedoMapPath", "");
            material.normalMapPath = materialJson.value("normalMapPath", "");
            material.specularMapPath = materialJson.value("specularMapPath", "");
            material.emissiveMapPath = materialJson.value("emissiveMapPath", "");
            material.aoMapPath = materialJson.value("aoMapPath", "");
            registry.add<MaterialComponent>(entity, material);
        }

        if (entityJson.contains("directionalLight")) {
            const json& lightJson = entityJson.at("directionalLight");
            DirectionalLightComponent component;
            component.light.direction = vec3FromJson(lightJson.at("direction"));
            component.light.color = vec3FromJson(lightJson.at("color"));
            component.light.intensity = lightJson.at("intensity").get<float>();
            component.light.enabled = lightJson.value("enabled", true);
            registry.add<DirectionalLightComponent>(entity, component);
        }

        if (entityJson.contains("pointLight")) {
            const json& lightJson = entityJson.at("pointLight");
            PointLightComponent component;
            component.light.position = vec3FromJson(lightJson.at("position"));
            component.light.color = vec3FromJson(lightJson.at("color"));
            component.light.intensity = lightJson.at("intensity").get<float>();
            component.light.constantAttenuation = lightJson.value("constantAttenuation", 1.0f);
            component.light.linearAttenuation = lightJson.value("linearAttenuation", 0.09f);
            component.light.quadraticAttenuation = lightJson.value("quadraticAttenuation", 0.032f);
            registry.add<PointLightComponent>(entity, component);
            // Keep entity transform aligned with the light for gizmo/picking.
            registry.get<TransformComponent>(entity).transform.setPosition(
                component.light.position);
        }

        if (entityJson.contains("spotLight")) {
            const json& lightJson = entityJson.at("spotLight");
            SpotLightComponent component;
            component.light.position = vec3FromJson(lightJson.at("position"));
            component.light.direction = vec3FromJson(lightJson.at("direction"));
            component.light.color = vec3FromJson(lightJson.at("color"));
            component.light.intensity = lightJson.at("intensity").get<float>();
            component.light.innerCutoffDegrees = lightJson.value("innerCutoffDegrees", 12.5f);
            component.light.outerCutoffDegrees = lightJson.value("outerCutoffDegrees", 17.5f);
            component.light.constantAttenuation = lightJson.value("constantAttenuation", 1.0f);
            component.light.linearAttenuation = lightJson.value("linearAttenuation", 0.09f);
            component.light.quadraticAttenuation = lightJson.value("quadraticAttenuation", 0.032f);
            component.light.enabled = lightJson.value("enabled", true);
            component.followCamera = lightJson.value("followCamera", false);
            registry.add<SpotLightComponent>(entity, component);
            if (!component.followCamera) {
                registry.get<TransformComponent>(entity).transform.setPosition(
                    component.light.position);
            }
        }

        if (entityJson.contains("hemisphereLight")) {
            const json& lightJson = entityJson.at("hemisphereLight");
            HemisphereLightComponent component;
            component.light.skyColor = vec3FromJson(lightJson.at("skyColor"));
            component.light.groundColor = vec3FromJson(lightJson.at("groundColor"));
            component.light.intensity = lightJson.at("intensity").get<float>();
            if (lightJson.contains("upDirection")) {
                component.light.upDirection = vec3FromJson(lightJson.at("upDirection"));
            }
            component.light.enabled = lightJson.value("enabled", true);
            registry.add<HemisphereLightComponent>(entity, component);
        }

        if (entityJson.contains("rigidBody")) {
            const json& bodyJson = entityJson.at("rigidBody");
            RigidBodyComponent body;
            body.motionType = static_cast<PhysicsMotionType>(
                bodyJson.value("motionType", static_cast<int>(PhysicsMotionType::Dynamic)));
            body.mass = bodyJson.value("mass", 1.0f);
            body.friction = bodyJson.value("friction", 0.5f);
            body.restitution = bodyJson.value("restitution", 0.0f);
            body.enableGravity = bodyJson.value("enableGravity", true);
            body.enabled = bodyJson.value("enabled", true);
            body.needsRebuild = true;
            registry.add<RigidBodyComponent>(entity, body);
        }

        if (entityJson.contains("collider")) {
            const json& colliderJson = entityJson.at("collider");
            ColliderComponent collider;
            collider.shape = static_cast<ColliderShapeType>(
                colliderJson.value("shape", static_cast<int>(ColliderShapeType::Box)));
            if (colliderJson.contains("boxHalfExtents")) {
                collider.boxHalfExtents = vec3FromJson(colliderJson.at("boxHalfExtents"));
            }
            collider.sphereRadius = colliderJson.value("sphereRadius", 0.5f);
            collider.capsuleRadius = colliderJson.value("capsuleRadius", 0.25f);
            collider.capsuleHalfHeight = colliderJson.value("capsuleHalfHeight", 0.5f);
            if (colliderJson.contains("localOffset")) {
                collider.localOffset = vec3FromJson(colliderJson.at("localOffset"));
            }
            registry.add<ColliderComponent>(entity, collider);
        }
    }

    if (root.contains("selectedEntity")) {
        m_scene.setSelectedEntity(root.at("selectedEntity").get<Entity>());
    }
}

} // namespace Engine
