#pragma once

#include "Editor/Panels/CameraPanel.h"
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Panels/HierarchyPanel.h"
#include "Editor/Panels/InspectorPanel.h"
#include "Editor/Panels/PerformancePanel.h"
#include "Editor/Viewport/EditorCameraController.h"
#include "Editor/Viewport/ViewportGizmo.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Core/Window.h"
#include "Engine/Physics/PhysicsSystem.h"
#include "Engine/Scene/Scene.h"

#include <filesystem>
#include <string>

namespace Editor {

class EditorLayer final {
public:
    void onImGuiRender(
        Engine::Window& window,
        Engine::Scene& scene,
        Engine::AssetManager& assets,
        Engine::PerspectiveCamera& camera,
        EditorCameraController& editorCamera,
        Engine::PhysicsSystem& physics,
        float deltaTime,
        float viewportWidth,
        float viewportHeight);

    void setScenePath(const std::filesystem::path& path);
    [[nodiscard]] const std::filesystem::path& scenePath() const;
    [[nodiscard]] bool isDirty() const;
    void markClean();

    [[nodiscard]] ViewportGizmo& gizmo();
    [[nodiscard]] const ViewportGizmo& gizmo() const;

private:
    void updateWindowTitle(Engine::Window& window) const;
    void newScene(Engine::Scene& scene, Engine::PhysicsSystem& physics);
    void openScene(
        Engine::Scene& scene,
        Engine::AssetManager& assets,
        Engine::PhysicsSystem& physics,
        const std::filesystem::path& path);
    void saveScene(Engine::Scene& scene);
    void saveSceneAs(Engine::Scene& scene, const std::string& fileName);

    HierarchyPanel m_hierarchy;
    InspectorPanel m_inspector;
    ContentBrowserPanel m_contentBrowser;
    CameraPanel m_cameraPanel;
    PerformancePanel m_performancePanel;
    ViewportGizmo m_gizmo;

    std::filesystem::path m_scenePath{"Assets/Scenes/default.json"};
    bool m_dirty = false;
    std::string m_openSceneRequest;
    bool m_showSaveAsPopup = false;
    char m_saveAsName[128] = "NewScene";
};

} // namespace Editor
