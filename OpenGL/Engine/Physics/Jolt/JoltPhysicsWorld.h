#pragma once

#include "Engine/Physics/IPhysicsWorld.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace Engine {

class JoltPhysicsWorld final : public IPhysicsWorld {
public:
    JoltPhysicsWorld();
    ~JoltPhysicsWorld() override;

    void setGravity(const glm::vec3& gravity) override;
    [[nodiscard]] glm::vec3 gravity() const override;

    PhysicsBodyId createBody(const PhysicsBodyDesc& desc) override;
    void destroyBody(PhysicsBodyId bodyId) override;
    void clear() override;

    void setBodyTransform(
        PhysicsBodyId bodyId,
        const glm::vec3& position,
        const glm::vec3& rotationDegrees) override;
    void getBodyTransform(
        PhysicsBodyId bodyId,
        glm::vec3& outPosition,
        glm::vec3& outRotationDegrees) const override;

    void setBodyMotionType(PhysicsBodyId bodyId, PhysicsMotionType motion) override;
    void addImpulse(PhysicsBodyId bodyId, const glm::vec3& impulse) override;

    void step(float deltaTime) override;

private:
    [[nodiscard]] JPH::BodyID toJoltId(PhysicsBodyId bodyId) const;
    [[nodiscard]] bool isValid(PhysicsBodyId bodyId) const;

    std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
    std::unique_ptr<JPH::JobSystemThreadPool> m_jobSystem;
    std::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;

    std::unordered_map<PhysicsBodyId, JPH::BodyID> m_bodies;
    PhysicsBodyId m_nextBodyId = 1;
    glm::vec3 m_gravity{0.0f, -9.81f, 0.0f};
    float m_accumulator = 0.0f;
    static constexpr float FixedDelta = 1.0f / 60.0f;
};

} // namespace Engine
