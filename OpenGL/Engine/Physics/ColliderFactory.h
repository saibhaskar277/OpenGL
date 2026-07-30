#pragma once

#include "Engine/Physics/PhysicsComponents.h"

#include <glm/glm.hpp>

namespace Engine {

// Helpers to build collider descriptions without touching mesh assets.
class ColliderFactory final {
public:
    ColliderFactory() = delete;

    [[nodiscard]] static ColliderComponent boxFromScale(const glm::vec3& scale);
    [[nodiscard]] static ColliderComponent box(const glm::vec3& halfExtents);
    [[nodiscard]] static ColliderComponent sphere(float radius);
    [[nodiscard]] static ColliderComponent capsule(float radius, float halfHeight);

    [[nodiscard]] static ColliderShapeDesc toShapeDesc(const ColliderComponent& collider);
};

} // namespace Engine
