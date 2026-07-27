#pragma once

#include "Engine/Renderer/Texture.h"

#include <glm/glm.hpp>

#include <memory>

namespace Engine {

class Shader;

struct Material {
    glm::vec3 albedo{1.0f};
    glm::vec3 specularColor{0.5f};
    float shininess = 32.0f;
    glm::vec3 emissive{0.0f};
    float ambientOcclusion = 1.0f;

    std::shared_ptr<Texture> albedoMap;
    std::shared_ptr<Texture> normalMap;
    std::shared_ptr<Texture> specularMap;
    std::shared_ptr<Texture> emissiveMap;
    std::shared_ptr<Texture> aoMap;

    // Binds maps starting at firstUnit.
    // Layout: albedo, normal, specular, emissive, ao
    void bindMaps(Shader& shader, unsigned int firstUnit = 1) const;
};

} // namespace Engine
