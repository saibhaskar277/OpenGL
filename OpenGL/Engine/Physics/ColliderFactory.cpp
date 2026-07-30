#include "Engine/Physics/ColliderFactory.h"

#include <algorithm>
#include <cmath>

namespace Engine {

ColliderComponent ColliderFactory::boxFromScale(const glm::vec3& scale) {
    ColliderComponent collider;
    collider.shape = ColliderShapeType::Box;
    collider.boxHalfExtents = glm::vec3(
        std::max(std::abs(scale.x) * 0.5f, 0.01f),
        std::max(std::abs(scale.y) * 0.5f, 0.01f),
        std::max(std::abs(scale.z) * 0.5f, 0.01f));
    return collider;
}

ColliderComponent ColliderFactory::box(const glm::vec3& halfExtents) {
    ColliderComponent collider;
    collider.shape = ColliderShapeType::Box;
    collider.boxHalfExtents = halfExtents;
    return collider;
}

ColliderComponent ColliderFactory::sphere(float radius) {
    ColliderComponent collider;
    collider.shape = ColliderShapeType::Sphere;
    collider.sphereRadius = std::max(radius, 0.05f);
    return collider;
}

ColliderComponent ColliderFactory::capsule(float radius, float halfHeight) {
    ColliderComponent collider;
    collider.shape = ColliderShapeType::Capsule;
    collider.capsuleRadius = std::max(radius, 0.05f);
    collider.capsuleHalfHeight = std::max(halfHeight, 0.05f);
    return collider;
}

ColliderShapeDesc ColliderFactory::toShapeDesc(const ColliderComponent& collider) {
    ColliderShapeDesc desc;
    desc.type = collider.shape;
    desc.boxHalfExtents = collider.boxHalfExtents;
    desc.sphereRadius = collider.sphereRadius;
    desc.capsuleRadius = collider.capsuleRadius;
    desc.capsuleHalfHeight = collider.capsuleHalfHeight;
    desc.localOffset = collider.localOffset;
    return desc;
}

} // namespace Engine
