#include "Editor/Panels/PerformancePanel.h"

#include "Engine/Scene/Components.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>

namespace Editor {

void PerformancePanel::beginFrame() {
}

void PerformancePanel::onImGuiRender(const Engine::Scene& scene, float deltaTime) {
    const float frameMs = std::max(deltaTime * 1000.0f, 0.0001f);
    const float fps = 1000.0f / frameMs;

    const float smooth = 0.1f;
    m_smoothedFrameMs = m_smoothedFrameMs <= 0.0f
        ? frameMs
        : (m_smoothedFrameMs * (1.0f - smooth) + frameMs * smooth);
    m_smoothedFps = m_smoothedFps <= 0.0f
        ? fps
        : (m_smoothedFps * (1.0f - smooth) + fps * smooth);

    m_minFrameMs = std::min(m_minFrameMs, frameMs);
    m_maxFrameMs = std::max(m_maxFrameMs, frameMs);
    m_resetTimer += deltaTime;
    if (m_resetTimer > 2.0f) {
        m_minFrameMs = frameMs;
        m_maxFrameMs = frameMs;
        m_resetTimer = 0.0f;
    }

    const Engine::Registry& registry = scene.registry();
    const int entityCount = static_cast<int>(registry.entities().size());
    int meshCount = 0;
    int pointLights = 0;
    int otherLights = 0;
    for (const auto& [entity, mesh] : registry.storage<Engine::MeshRendererComponent>()) {
        (void)entity;
        if (mesh.visible) {
            ++meshCount;
        }
    }
    for (const auto& [entity, light] : registry.storage<Engine::PointLightComponent>()) {
        (void)entity;
        (void)light;
        ++pointLights;
    }
    for (const auto& [entity, light] : registry.storage<Engine::DirectionalLightComponent>()) {
        (void)entity;
        (void)light;
        ++otherLights;
    }
    for (const auto& [entity, light] : registry.storage<Engine::SpotLightComponent>()) {
        (void)entity;
        (void)light;
        ++otherLights;
    }
    for (const auto& [entity, light] : registry.storage<Engine::HemisphereLightComponent>()) {
        (void)entity;
        (void)light;
        ++otherLights;
    }

    const float menuBarHeight = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(10.0f, menuBarHeight + 8.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.72f);
    ImGui::Begin(
        "##PerformanceOverlay",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav |
            ImGuiWindowFlags_NoMove);

    ImGui::Text("Performance");
    ImGui::Separator();
    ImGui::Text("FPS:  %.1f", m_smoothedFps);
    ImGui::Text("Frame: %.2f ms  (min %.2f / max %.2f)", m_smoothedFrameMs, m_minFrameMs, m_maxFrameMs);
    ImGui::Separator();
    ImGui::Text("Entities: %d", entityCount);
    ImGui::Text("Visible meshes: %d", meshCount);
    ImGui::Text("Lights: %d point + %d other", pointLights, otherLights);

    const Engine::Entity selected = scene.selectedEntity();
    if (selected != Engine::NullEntity && registry.alive(selected)) {
        const char* name = registry.has<Engine::TagComponent>(selected)
            ? registry.get<Engine::TagComponent>(selected).name.c_str()
            : "Entity";
        ImGui::Text("Selected: %s (#%u)", name, static_cast<unsigned>(selected));
    } else {
        ImGui::TextUnformatted("Selected: (none)");
    }

    ImGui::End();
}

} // namespace Editor
