#pragma once

#include "Engine/Renderer/Lighting/DirectionalLight.h"
#include "Engine/Renderer/Lighting/HemisphereLight.h"
#include "Engine/Renderer/Lighting/PointLight.h"
#include "Engine/Renderer/Lighting/SpotLight.h"
#include "Engine/Scene/Transform.h"
#include "Engine/Physics/PhysicsComponents.h"

#include <glm/glm.hpp>

#include <string>

namespace Engine {

struct TagComponent {
    std::string name = "Entity";
};

struct TransformComponent {
    Transform transform;
};

enum class MeshType {
    Cube = 0,
    Model = 1
};

struct MeshRendererComponent {
    MeshType meshType = MeshType::Cube;
    std::string modelPath;
    bool castShadows = true;
    bool visible = true;
};

struct MaterialComponent {
    glm::vec3 albedo{1.0f};
    glm::vec3 specularColor{0.5f};
    float shininess = 32.0f;
    glm::vec3 emissive{0.0f};
    float ambientOcclusion = 1.0f;

    std::string albedoMapPath;
    std::string normalMapPath;
    std::string specularMapPath;
    std::string emissiveMapPath;
    std::string aoMapPath;
};

struct DirectionalLightComponent {
    DirectionalLight light;
};

struct PointLightComponent {
    PointLight light;
};

struct SpotLightComponent {
    SpotLight light;
    bool followCamera = false;
};

struct HemisphereLightComponent {
    HemisphereLight light;
};

} // namespace Engine
