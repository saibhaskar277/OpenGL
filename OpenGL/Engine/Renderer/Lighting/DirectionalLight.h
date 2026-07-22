#pragma once

#include <glm/glm.hpp>

namespace Engine {

struct DirectionalLight {
    glm::vec3 direction{-0.4f, -1.0f, -0.3f};
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    bool enabled = true;
};

} // namespace Engine
