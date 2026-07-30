#pragma once

#include "Engine/Physics/IPhysicsWorld.h"
#include "Engine/Scene/Scene.h"

#include <unordered_map>

namespace Engine {

// Bridges ECS components <-> IPhysicsWorld. Keeps Jolt out of Scene/Renderer.
class PhysicsSystem final {
public:
    explicit PhysicsSystem(IPhysicsWorld& world);

    void setSimulationEnabled(bool enabled);
    [[nodiscard]] bool simulationEnabled() const;

    // Call after scene load / clear.
    void rebuild(Scene& scene);

    // Per-frame: push editor transforms, step (if enabled), pull dynamic poses.
    void update(Scene& scene, float deltaTime, bool transformsAreAuthoritative);

private:
    void ensureBodies(Scene& scene);
    void syncTransformsToBodies(Scene& scene);
    void syncBodiesToTransforms(Scene& scene);

    IPhysicsWorld& m_world;
    bool m_simulationEnabled = false;
    std::unordered_map<Entity, PhysicsBodyId> m_entityBodies;
};

} // namespace Engine
