#pragma once

#include "Engine/Camera/FlyCameraController.h"
#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Core/Window.h"
#include "Engine/Scene/Scene.h"

#include <glm/glm.hpp>

namespace Editor {

// Unity / Maya / Unreal-style editor navigation:
// - RMB + WASD: fly
// - Alt+LMB: orbit around pivot
// - Alt+MMB (or Alt+Shift+LMB): pan
// - Alt+RMB / mouse wheel: dolly zoom
// - F: focus selected entity
class EditorCameraController final {
public:
    void update(
        Engine::Window& window,
        Engine::PerspectiveCamera& camera,
        Engine::Scene& scene,
        float deltaTime,
        bool captureMouse,
        bool captureKeyboard);

    void focusSelected(Engine::PerspectiveCamera& camera, Engine::Scene& scene);
    void resetPivot(const glm::vec3& pivot = glm::vec3(0.0f));

    [[nodiscard]] Engine::FlyCameraController& fly();
    [[nodiscard]] const Engine::FlyCameraController& fly() const;
    [[nodiscard]] const glm::vec3& pivot() const;

private:
    void beginMouseSample(Engine::Window& window);
    bool sampleMouseDelta(Engine::Window& window, float& outDx, float& outDy);
    void orbit(Engine::PerspectiveCamera& camera, float yawDelta, float pitchDelta);
    void pan(Engine::PerspectiveCamera& camera, float dx, float dy);
    void dolly(Engine::PerspectiveCamera& camera, float amount);

    Engine::FlyCameraController m_fly;
    glm::vec3 m_pivot{0.0f, 0.0f, 0.0f};
    double m_previousMouseX = 0.0;
    double m_previousMouseY = 0.0;
    bool m_firstMouseSample = true;
    float m_orbitSensitivity = 0.25f;
    float m_panSensitivity = 0.0025f;
    float m_zoomSensitivity = 0.12f;
};

} // namespace Editor
