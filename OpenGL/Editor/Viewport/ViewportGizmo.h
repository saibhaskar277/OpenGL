#pragma once

#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Scene/Scene.h"

namespace Editor {

enum class GizmoMode {
    Translate = 0,
    Rotate = 1,
    Scale = 2
};

class ViewportGizmo final {
public:
    void beginFrame();
    // Returns true if the gizmo is being used (blocks picking).
    bool draw(
        Engine::Scene& scene,
        const Engine::PerspectiveCamera& camera,
        float viewportWidth,
        float viewportHeight);

    void setMode(GizmoMode mode);
    [[nodiscard]] GizmoMode mode() const;
    [[nodiscard]] bool isUsing() const;
    [[nodiscard]] bool isOver() const;

private:
    GizmoMode m_mode = GizmoMode::Translate;
    bool m_using = false;
    bool m_over = false;
};

} // namespace Editor
