#pragma once

#include <glm/glm.hpp>

namespace Engine {

// Classic hemisphere ambient fill (OpenGL / GPU Gems style):
// blend = 0.5 * (1 + N·up); color = mix(ground, sky, blend) * intensity
struct HemisphereLight {
    glm::vec3 skyColor{0.35f, 0.45f, 0.65f};
    glm::vec3 groundColor{0.22f, 0.16f, 0.10f};
    float intensity = 0.35f;
    glm::vec3 upDirection{0.0f, 1.0f, 0.0f};
    bool enabled = true;
};

} // namespace Engine
