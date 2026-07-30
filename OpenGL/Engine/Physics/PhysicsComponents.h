#pragma once

#include "Engine/Physics/PhysicsTypes.h"

#include <glm/glm.hpp>

namespace Engine {

struct RigidBodyComponent {
    PhysicsMotionType motionType = PhysicsMotionType::Dynamic;
    float mass = 1.0f;
    float friction = 0.5f;
    float restitution = 0.0f;
    bool enableGravity = true;
    bool enabled = true;

    // Runtime only (not serialized).
    PhysicsBodyId bodyId = InvalidPhysicsBodyId;
    bool needsRebuild = true;
};

struct ColliderComponent {
    ColliderShapeType shape = ColliderShapeType::Box;
    glm::vec3 boxHalfExtents{0.5f};
    float sphereRadius = 0.5f;
    float capsuleRadius = 0.25f;
    float capsuleHalfHeight = 0.5f;
    glm::vec3 localOffset{0.0f};

    // Runtime only (not serialized). Used for collider-only static bodies.
    bool needsRebuild = true;
};

} // namespace Engine
