#pragma once

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Scene/Scene.h"

namespace Editor {

// Wireframe collider shapes in the viewport (box / sphere / capsule).
void drawColliderDebugGizmos(
    Engine::Scene& scene,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight);

} // namespace Editor
