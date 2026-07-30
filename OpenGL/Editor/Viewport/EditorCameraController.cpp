#include "Editor/Viewport/EditorCameraController.h"

#include "Engine/Scene/Components.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace Editor {
namespace {

float distanceToPivot(const Engine::PerspectiveCamera& camera, const glm::vec3& pivot) {
    return std::max(glm::length(camera.position() - pivot), 0.15f);
}

} // namespace

void EditorCameraController::beginMouseSample(Engine::Window& window) {
    double mouseX = 0.0;
    double mouseY = 0.0;
    window.cursorPosition(mouseX, mouseY);
    m_previousMouseX = mouseX;
    m_previousMouseY = mouseY;
    m_firstMouseSample = false;
}

bool EditorCameraController::sampleMouseDelta(
    Engine::Window& window,
    float& outDx,
    float& outDy) {
    double mouseX = 0.0;
    double mouseY = 0.0;
    window.cursorPosition(mouseX, mouseY);

    if (m_firstMouseSample) {
        m_previousMouseX = mouseX;
        m_previousMouseY = mouseY;
        m_firstMouseSample = false;
        outDx = 0.0f;
        outDy = 0.0f;
        return false;
    }

    outDx = static_cast<float>(mouseX - m_previousMouseX);
    outDy = static_cast<float>(mouseY - m_previousMouseY);
    m_previousMouseX = mouseX;
    m_previousMouseY = mouseY;
    return true;
}

void EditorCameraController::orbit(
    Engine::PerspectiveCamera& camera,
    float yawDelta,
    float pitchDelta) {
    const float distance = distanceToPivot(camera, m_pivot);
    const float yaw = camera.yawDegrees() + yawDelta;
    const float pitch = std::clamp(camera.pitchDegrees() + pitchDelta, -89.0f, 89.0f);
    camera.setOrientation(yaw, pitch);
    camera.setPosition(m_pivot - camera.forward() * distance);
}

void EditorCameraController::pan(Engine::PerspectiveCamera& camera, float dx, float dy) {
    const float distance = distanceToPivot(camera, m_pivot);
    const float scale = distance * m_panSensitivity;
    const glm::vec3 offset = (-camera.right() * dx + camera.up() * dy) * scale;
    camera.translate(offset);
    m_pivot += offset;
}

void EditorCameraController::dolly(Engine::PerspectiveCamera& camera, float amount) {
    float distance = distanceToPivot(camera, m_pivot);
    distance = std::clamp(distance * (1.0f - amount * m_zoomSensitivity), 0.15f, 500.0f);
    camera.setPosition(m_pivot - camera.forward() * distance);
}

void EditorCameraController::focusSelected(
    Engine::PerspectiveCamera& camera,
    Engine::Scene& scene) {
    const Engine::Entity selected = scene.selectedEntity();
    if (selected == Engine::NullEntity || !scene.registry().alive(selected)) {
        return;
    }
    if (!scene.registry().has<Engine::TransformComponent>(selected)) {
        return;
    }

    const Engine::Transform& transform =
        scene.registry().get<Engine::TransformComponent>(selected).transform;
    m_pivot = transform.position();

    const glm::vec3 scale = transform.scale();
    const float radius = std::max({std::abs(scale.x), std::abs(scale.y), std::abs(scale.z), 1.0f});
    const float distance = std::clamp(radius * 2.75f, 1.5f, 80.0f);
    camera.setPosition(m_pivot - camera.forward() * distance);
}

void EditorCameraController::resetPivot(const glm::vec3& pivot) {
    m_pivot = pivot;
}

Engine::FlyCameraController& EditorCameraController::fly() {
    return m_fly;
}

const Engine::FlyCameraController& EditorCameraController::fly() const {
    return m_fly;
}

const glm::vec3& EditorCameraController::pivot() const {
    return m_pivot;
}

void EditorCameraController::update(
    Engine::Window& window,
    Engine::PerspectiveCamera& camera,
    Engine::Scene& scene,
    float deltaTime,
    bool captureMouse,
    bool captureKeyboard) {
    GLFWwindow* glfwWindow = window.nativeHandle();
    const bool alt =
        glfwGetKey(glfwWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
    const bool shift =
        glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(glfwWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
    const bool left =
        glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    const bool middle =
        glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    const bool right =
        glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    if (!captureKeyboard && ImGui::IsKeyPressed(ImGuiKey_F, false)) {
        focusSelected(camera, scene);
    }

    // Mouse wheel zoom (Unity / Maya style) when viewport has focus.
    if (!captureMouse) {
        const float wheel = ImGui::GetIO().MouseWheel;
        if (std::abs(wheel) > 0.0f) {
            dolly(camera, wheel);
        }
    }

    // Alt navigation (Maya / Unreal / Unity).
    if (!captureMouse && alt && (left || middle || right)) {
        float dx = 0.0f;
        float dy = 0.0f;
        if (sampleMouseDelta(window, dx, dy)) {
            if (middle || (left && shift)) {
                pan(camera, dx, dy);
            } else if (left) {
                orbit(camera, dx * m_orbitSensitivity, -dy * m_orbitSensitivity);
            } else if (right) {
                dolly(camera, -dy * 0.05f);
            }
        }
        return;
    }

    // Keep a useful orbit pivot in front while flying.
    if (!captureMouse && !captureKeyboard && right && !alt) {
        window.setCursorCaptured(true);
        m_fly.update(window, camera, deltaTime);
        m_pivot = camera.position() + camera.forward() * 4.0f;
        m_firstMouseSample = true;
        return;
    }

    window.setCursorCaptured(false);
    m_fly.resetMouseTracking();
    m_firstMouseSample = true;
}

} // namespace Editor
