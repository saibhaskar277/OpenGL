#include "Editor/App/EditorLayer.h"

#include "Editor/Viewport/LightDebugOverlay.h"
#include "Engine/Scene/SceneSerializer.h"

#include <imgui.h>

namespace Editor {

void EditorLayer::onImGuiRender(
    Engine::Scene& scene,
    Engine::AssetManager& assets,
    Engine::PerspectiveCamera& camera,
    Engine::FlyCameraController& cameraController,
    float viewportWidth,
    float viewportHeight) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                scene.createDefaultScene();
            }
            if (ImGui::MenuItem("Save Scene")) {
                Engine::SceneSerializer(scene).save("Assets/Scenes/default.json");
            }
            if (ImGui::MenuItem("Load Scene")) {
                Engine::SceneSerializer(scene).load("Assets/Scenes/default.json");
                assets.refresh();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Add Cube")) {
                scene.setSelectedEntity(scene.createCube("Cube"));
            }
            if (ImGui::BeginMenu("Add Light")) {
                if (ImGui::MenuItem("Directional Light")) {
                    scene.setSelectedEntity(scene.createDirectionalLight());
                }
                if (ImGui::MenuItem("Point Light")) {
                    scene.setSelectedEntity(scene.createPointLight());
                }
                if (ImGui::MenuItem("Spot Light")) {
                    scene.setSelectedEntity(scene.createSpotLight());
                }
                if (ImGui::MenuItem("Hemisphere Light")) {
                    scene.setSelectedEntity(scene.createHemisphereLight());
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Gizmo")) {
            if (ImGui::MenuItem("Translate", "W", m_gizmo.mode() == GizmoMode::Translate)) {
                m_gizmo.setMode(GizmoMode::Translate);
            }
            if (ImGui::MenuItem("Rotate", "E", m_gizmo.mode() == GizmoMode::Rotate)) {
                m_gizmo.setMode(GizmoMode::Rotate);
            }
            if (ImGui::MenuItem("Scale", "R", m_gizmo.mode() == GizmoMode::Scale)) {
                m_gizmo.setMode(GizmoMode::Scale);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Avoid stealing W/E/R while flying the camera with RMB.
    const bool flying = ImGui::IsMouseDown(ImGuiMouseButton_Right);
    if (!flying && !ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) {
            m_gizmo.setMode(GizmoMode::Translate);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_E)) {
            m_gizmo.setMode(GizmoMode::Rotate);
        }
        if (ImGui::IsKeyPressed(ImGuiKey_R)) {
            m_gizmo.setMode(GizmoMode::Scale);
        }
    }

    drawLightDebugMarkers(scene, camera, viewportWidth, viewportHeight);

    m_gizmo.beginFrame();
    m_gizmo.draw(scene, camera, viewportWidth, viewportHeight);

    m_hierarchy.onImGuiRender(scene);
    m_inspector.onImGuiRender(scene, assets);
    m_contentBrowser.onImGuiRender(assets);
    m_cameraPanel.onImGuiRender(camera, cameraController);
}

ViewportGizmo& EditorLayer::gizmo() {
    return m_gizmo;
}

const ViewportGizmo& EditorLayer::gizmo() const {
    return m_gizmo;
}

} // namespace Editor
