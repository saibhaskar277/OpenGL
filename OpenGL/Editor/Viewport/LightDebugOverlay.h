#pragma once

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Scene/Scene.h"

namespace Editor {

// Draws clickable debug markers for lights in the viewport.
void drawLightDebugMarkers(
    Engine::Scene& scene,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight);

} // namespace Editor
