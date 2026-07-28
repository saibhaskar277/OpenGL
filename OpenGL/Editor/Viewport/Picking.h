#pragma once

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Scene.h"

namespace Editor {

struct Ray {
    glm::vec3 origin{0.0f};
    glm::vec3 direction{0.0f, 0.0f, -1.0f};
};

Ray screenPointToRay(
    const Engine::PerspectiveCamera& camera,
    float mouseX,
    float mouseY,
    float viewportWidth,
    float viewportHeight);

// Left-click picking for meshes and light positions.
Engine::Entity pickEntity(
    Engine::Scene& scene,
    const Engine::PerspectiveCamera& camera,
    float mouseX,
    float mouseY,
    float viewportWidth,
    float viewportHeight);

} // namespace Editor
