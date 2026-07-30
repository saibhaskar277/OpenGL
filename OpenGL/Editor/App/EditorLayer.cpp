#include "Editor/App/EditorLayer.h"

#include "Editor/Viewport/LightDebugOverlay.h"
#include "Editor/Viewport/ColliderDebugOverlay.h"
#include "Engine/Scene/SceneSerializer.h"

#include <imgui.h>

#include <iostream>

namespace Editor {
namespace {

std::filesystem::path assetsRelativeScenePath(const std::string& request) {
    if (request.rfind("Assets/", 0) == 0) {
        return request;
    }
    return std::filesystem::path("Assets") / request;
}

std::string sceneDisplayName(const std::filesystem::path& path) {
    if (path.empty()) {
        return "Untitled";
    }
    return path.filename().string();
}

} // namespace

void EditorLayer::updateWindowTitle(Engine::Window& window) const {
    std::string title = "OpenGL Rendering Editor - " + sceneDisplayName(m_scenePath);
    if (m_dirty) {
        title += " *";
    }
    window.setTitle(title);
}

void EditorLayer::newScene(Engine::Scene& scene, Engine::PhysicsSystem& physics) {
    scene.clear();
    physics.rebuild(scene);
    m_scenePath.clear();
    m_dirty = true;
}

void EditorLayer::openScene(
    Engine::Scene& scene,
    Engine::AssetManager& assets,
    Engine::PhysicsSystem& physics,
    const std::filesystem::path& path) {
    try {
        Engine::SceneSerializer(scene).load(path);
        physics.rebuild(scene);
        m_scenePath = path;
        m_dirty = false;
        assets.refresh();
    } catch (const std::exception& error) {
        std::cerr << "Failed to open scene '" << path.string() << "': "
                  << error.what() << '\n';
    }
}

void EditorLayer::saveScene(Engine::Scene& scene) {
    if (m_scenePath.empty()) {
        m_showSaveAsPopup = true;
        return;
    }
    try {
        Engine::SceneSerializer(scene).save(m_scenePath);
        m_dirty = false;
    } catch (const std::exception& error) {
        std::cerr << "Failed to save scene: " << error.what() << '\n';
    }
}

void EditorLayer::saveSceneAs(Engine::Scene& scene, const std::string& fileName) {
    std::string name = fileName;
    if (name.empty()) {
        return;
    }
    if (name.find('.') == std::string::npos) {
        name += ".json";
    }
    const std::filesystem::path path = std::filesystem::path("Assets/Scenes") / name;
    try {
        Engine::SceneSerializer(scene).save(path);
        m_scenePath = path;
        m_dirty = false;
    } catch (const std::exception& error) {
        std::cerr << "Failed to save scene as: " << error.what() << '\n';
    }
}

void EditorLayer::onImGuiRender(
    Engine::Window& window,
    Engine::Scene& scene,
    Engine::AssetManager& assets,
    Engine::PerspectiveCamera& camera,
    EditorCameraController& editorCamera,
    Engine::PhysicsSystem& physics,
    float deltaTime,
    float viewportWidth,
    float viewportHeight) {
    m_openSceneRequest.clear();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                newScene(scene, physics);
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                saveScene(scene);
            }
            if (ImGui::MenuItem("Save As...")) {
                m_showSaveAsPopup = true;
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Open Scene")) {
                for (const Engine::SceneAssetInfo& sceneAsset : assets.scenes()) {
                    if (ImGui::MenuItem(sceneAsset.displayName.c_str())) {
                        m_openSceneRequest = sceneAsset.id;
                    }
                }
                if (assets.scenes().empty()) {
                    ImGui::TextDisabled("No scenes in Assets/Scenes");
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Add Cube")) {
                scene.setSelectedEntity(scene.createCube("Cube"));
                m_dirty = true;
            }
            if (ImGui::MenuItem("Add Physics Cube")) {
                scene.setSelectedEntity(scene.createPhysicsCube());
                m_dirty = true;
            }
            if (ImGui::BeginMenu("Add Light")) {
                if (ImGui::MenuItem("Directional Light")) {
                    scene.setSelectedEntity(scene.createDirectionalLight());
                    m_dirty = true;
                }
                if (ImGui::MenuItem("Point Light")) {
                    scene.setSelectedEntity(scene.createPointLight());
                    m_dirty = true;
                }
                if (ImGui::MenuItem("Spot Light")) {
                    scene.setSelectedEntity(scene.createSpotLight());
                    m_dirty = true;
                }
                if (ImGui::MenuItem("Hemisphere Light")) {
                    scene.setSelectedEntity(scene.createHemisphereLight());
                    m_dirty = true;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Focus Selected", "F")) {
                editorCamera.focusSelected(camera, scene);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Physics")) {
            bool simulating = physics.simulationEnabled();
            if (ImGui::MenuItem("Simulate", nullptr, simulating)) {
                physics.setSimulationEnabled(!simulating);
            }
            if (ImGui::MenuItem("Rebuild Bodies")) {
                physics.rebuild(scene);
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

    const ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        saveScene(scene);
    }
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
        newScene(scene, physics);
    }

    // Avoid stealing W/E/R while flying the camera with RMB.
    const bool flying = ImGui::IsMouseDown(ImGuiMouseButton_Right);
    if (!flying && !io.WantTextInput) {
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

    m_performancePanel.onImGuiRender(scene, deltaTime);

    drawLightDebugMarkers(scene, camera, viewportWidth, viewportHeight);
    drawColliderDebugGizmos(scene, camera, viewportWidth, viewportHeight);

    m_gizmo.beginFrame();
    if (m_gizmo.draw(scene, camera, viewportWidth, viewportHeight) && m_gizmo.isUsing()) {
        m_dirty = true;
    }

    if (m_hierarchy.onImGuiRender(scene)) {
        m_dirty = true;
    }
    if (m_inspector.onImGuiRender(scene, assets)) {
        m_dirty = true;
    }
    m_contentBrowser.onImGuiRender(assets, &m_openSceneRequest);
    m_cameraPanel.onImGuiRender(camera, editorCamera.fly());

    if (!m_openSceneRequest.empty()) {
        openScene(scene, assets, physics, assetsRelativeScenePath(m_openSceneRequest));
        m_openSceneRequest.clear();
    }

    if (m_showSaveAsPopup) {
        ImGui::OpenPopup("Save Scene As");
        m_showSaveAsPopup = false;
    }
    if (ImGui::BeginPopupModal("Save Scene As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("File name", m_saveAsName, sizeof(m_saveAsName));
        ImGui::TextDisabled("Saved to Assets/Scenes/");
        if (ImGui::Button("Save")) {
            saveSceneAs(scene, m_saveAsName);
            assets.refresh();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    updateWindowTitle(window);
}

void EditorLayer::setScenePath(const std::filesystem::path& path) {
    m_scenePath = path;
}

const std::filesystem::path& EditorLayer::scenePath() const {
    return m_scenePath;
}

bool EditorLayer::isDirty() const {
    return m_dirty;
}

void EditorLayer::markClean() {
    m_dirty = false;
}

ViewportGizmo& EditorLayer::gizmo() {
    return m_gizmo;
}

const ViewportGizmo& EditorLayer::gizmo() const {
    return m_gizmo;
}

} // namespace Editor
