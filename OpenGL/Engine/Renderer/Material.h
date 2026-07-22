#pragma once

#include <glm/glm.hpp>

namespace Engine {

struct Material {
    glm::vec3 albedo{1.0f};
    glm::vec3 specularColor{0.5f};
    float shininess = 32.0f;
};

} // namespace Engine
