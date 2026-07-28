#include "Engine/Camera/FlyCameraController.h"

#include "Engine/Core/Window.h"
#include "Engine/Camera/PerspectiveCamera.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <algorithm>

namespace Engine {

FlyCameraController::FlyCameraController(float movementSpeed, float mouseSensitivity)
    : m_movementSpeed(movementSpeed),
      m_mouseSensitivity(mouseSensitivity) {
}

void FlyCameraController::update(
    Window& window,
    PerspectiveCamera& camera,
    float deltaTime) {
    glm::vec3 movement(0.0f);

    if (window.isKeyPressed(GLFW_KEY_W)) {
        movement += camera.forward();
    }
    if (window.isKeyPressed(GLFW_KEY_S)) {
        movement -= camera.forward();
    }
    if (window.isKeyPressed(GLFW_KEY_D)) {
        movement += camera.right();
    }
    if (window.isKeyPressed(GLFW_KEY_A)) {
        movement -= camera.right();
    }
    if (window.isKeyPressed(GLFW_KEY_SPACE)) {
        movement.y += 1.0f;
    }
    if (window.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        movement.y -= 1.0f;
    }

    if (glm::dot(movement, movement) > 0.0f) {
        camera.translate(glm::normalize(movement) * m_movementSpeed * deltaTime);
    }

    double mouseX = 0.0;
    double mouseY = 0.0;
    window.cursorPosition(mouseX, mouseY);

    if (m_firstMouseUpdate) {
        m_previousMouseX = mouseX;
        m_previousMouseY = mouseY;
        m_firstMouseUpdate = false;
        return;
    }

    const float xOffset =
        static_cast<float>(mouseX - m_previousMouseX) * m_mouseSensitivity;
    const float yOffset =
        static_cast<float>(m_previousMouseY - mouseY) * m_mouseSensitivity;
    m_previousMouseX = mouseX;
    m_previousMouseY = mouseY;

    const float pitch = std::clamp(camera.pitchDegrees() + yOffset, -89.0f, 89.0f);
    camera.setOrientation(camera.yawDegrees() + xOffset, pitch);
}

void FlyCameraController::resetMouseTracking() {
    m_firstMouseUpdate = true;
}

void FlyCameraController::setMovementSpeed(float movementSpeed) {
    m_movementSpeed = movementSpeed;
}

void FlyCameraController::setMouseSensitivity(float mouseSensitivity) {
    m_mouseSensitivity = mouseSensitivity;
}

float FlyCameraController::movementSpeed() const {
    return m_movementSpeed;
}

float FlyCameraController::mouseSensitivity() const {
    return m_mouseSensitivity;
}

} // namespace Engine
