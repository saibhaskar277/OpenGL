#pragma once

#include "Editor/Panels/CameraPanel.h"
#include "Editor/Panels/ContentBrowserPanel.h"
#include "Editor/Panels/HierarchyPanel.h"
#include "Editor/Panels/InspectorPanel.h"
#include "Editor/Viewport/ViewportGizmo.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Camera/FlyCameraController.h"
#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Scene/Scene.h"

namespace Editor {

class EditorLayer final {
public:
    void onImGuiRender(
        Engine::Scene& scene,
        Engine::AssetManager& assets,
        Engine::PerspectiveCamera& camera,
        Engine::FlyCameraController& cameraController,
        float viewportWidth,
        float viewportHeight);

    [[nodiscard]] ViewportGizmo& gizmo();
    [[nodiscard]] const ViewportGizmo& gizmo() const;

private:
    HierarchyPanel m_hierarchy;
    InspectorPanel m_inspector;
    ContentBrowserPanel m_contentBrowser;
    CameraPanel m_cameraPanel;
    ViewportGizmo m_gizmo;
};

} // namespace Editor
