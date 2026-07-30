#pragma once

#include "Engine/Physics/PhysicsTypes.h"

#include <memory>

namespace Engine {

// Backend-agnostic physics world. Scene / editor depend on this, not Jolt.
class IPhysicsWorld {
public:
    virtual ~IPhysicsWorld() = default;

    virtual void setGravity(const glm::vec3& gravity) = 0;
    [[nodiscard]] virtual glm::vec3 gravity() const = 0;

    virtual PhysicsBodyId createBody(const PhysicsBodyDesc& desc) = 0;
    virtual void destroyBody(PhysicsBodyId bodyId) = 0;
    virtual void clear() = 0;

    virtual void setBodyTransform(
        PhysicsBodyId bodyId,
        const glm::vec3& position,
        const glm::vec3& rotationDegrees) = 0;
    virtual void getBodyTransform(
        PhysicsBodyId bodyId,
        glm::vec3& outPosition,
        glm::vec3& outRotationDegrees) const = 0;

    virtual void setBodyMotionType(PhysicsBodyId bodyId, PhysicsMotionType motion) = 0;
    virtual void addImpulse(PhysicsBodyId bodyId, const glm::vec3& impulse) = 0;

    // Fixed-step friendly; call with frame delta (world may substep internally).
    virtual void step(float deltaTime) = 0;
};

std::unique_ptr<IPhysicsWorld> createJoltPhysicsWorld();

} // namespace Engine
