#pragma once

#include <glm/glm.hpp>

namespace Engine {

struct SpotLight {
    glm::vec3 position{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
    float innerCutoffDegrees = 12.5f;
    float outerCutoffDegrees = 17.5f;
    float constantAttenuation = 1.0f;
    float linearAttenuation = 0.09f;
    float quadraticAttenuation = 0.032f;
    bool enabled = true;
};

} // namespace Engine
