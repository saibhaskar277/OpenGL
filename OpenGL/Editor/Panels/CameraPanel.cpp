#include "Editor/Panels/CameraPanel.h"

#include <imgui.h>

namespace Editor {

void CameraPanel::onImGuiRender(
    Engine::PerspectiveCamera& camera,
    Engine::FlyCameraController& controller) {
    ImGui::Begin("Camera");

    glm::vec3 position = camera.position();
    if (ImGui::DragFloat3("Position", &position.x, 0.05f)) {
        camera.setPosition(position);
    }

    float yaw = camera.yawDegrees();
    float pitch = camera.pitchDegrees();
    bool orientationChanged = false;
    orientationChanged |= ImGui::DragFloat("Yaw", &yaw, 0.5f);
    orientationChanged |= ImGui::DragFloat("Pitch", &pitch, 0.5f, -89.0f, 89.0f);
    if (orientationChanged) {
        camera.setOrientation(yaw, pitch);
    }

    float fov = camera.fieldOfViewDegrees();
    float nearClip = camera.nearClip();
    float farClip = camera.farClip();
    bool perspectiveChanged = false;
    perspectiveChanged |= ImGui::DragFloat("FOV", &fov, 0.1f, 10.0f, 120.0f);
    perspectiveChanged |= ImGui::DragFloat("Near", &nearClip, 0.01f, 0.01f, 10.0f);
    perspectiveChanged |= ImGui::DragFloat("Far", &farClip, 1.0f, 1.0f, 5000.0f);
    if (perspectiveChanged) {
        camera.setPerspective(fov, nearClip, farClip);
    }

    float speed = controller.movementSpeed();
    float sensitivity = controller.mouseSensitivity();
    if (ImGui::DragFloat("Move Speed", &speed, 0.05f, 0.1f, 50.0f)) {
        controller.setMovementSpeed(speed);
    }
    if (ImGui::DragFloat("Mouse Sensitivity", &sensitivity, 0.01f, 0.01f, 2.0f)) {
        controller.setMouseSensitivity(sensitivity);
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Controls");
    ImGui::BulletText("RMB + WASD / Space / Shift: fly");
    ImGui::BulletText("LMB: select object/light");
    ImGui::BulletText("W/E/R: translate / rotate / scale gizmo");

    if (ImGui::Button("Reset Camera")) {
        camera.setPosition(glm::vec3(0.0f, 1.2f, 4.5f));
        camera.setOrientation(-90.0f, -10.0f);
        camera.setPerspective(45.0f, 0.1f, 100.0f);
        controller.setMovementSpeed(3.0f);
        controller.setMouseSensitivity(0.1f);
    }

    ImGui::End();
}

} // namespace Editor
