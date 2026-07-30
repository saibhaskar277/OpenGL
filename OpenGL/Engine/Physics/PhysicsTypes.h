#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace Engine {

using PhysicsBodyId = std::uint32_t;
constexpr PhysicsBodyId InvalidPhysicsBodyId = static_cast<PhysicsBodyId>(-1);

enum class PhysicsMotionType {
    Static = 0,
    Kinematic = 1,
    Dynamic = 2
};

enum class ColliderShapeType {
    Box = 0,
    Sphere = 1,
    Capsule = 2
};

struct ColliderShapeDesc {
    ColliderShapeType type = ColliderShapeType::Box;
    glm::vec3 boxHalfExtents{0.5f};
    float sphereRadius = 0.5f;
    float capsuleRadius = 0.25f;
    float capsuleHalfHeight = 0.5f;
    glm::vec3 localOffset{0.0f};
};

struct PhysicsBodyDesc {
    PhysicsMotionType motion = PhysicsMotionType::Dynamic;
    float mass = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    bool enableGravity = true;
    glm::vec3 position{0.0f};
    glm::vec3 rotationDegrees{0.0f};
    ColliderShapeDesc collider;
    std::uint64_t userData = 0;
};

} // namespace Engine
