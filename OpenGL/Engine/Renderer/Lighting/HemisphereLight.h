#pragma once

#include <glm/glm.hpp>

namespace Engine {

struct HemisphereLight {
    glm::vec3 skyColor{0.35f, 0.45f, 0.65f};
    glm::vec3 groundColor{0.22f, 0.16f, 0.10f};
    float intensity = 0.3f;

    glm::vec3 bounceColor{1.0f, 0.35f, 0.12f};
    float bounceStrength = 0.35f;
    float floorHeight = -0.95f;
    float bounceFalloff = 1.5f;
};

} // namespace Engine
