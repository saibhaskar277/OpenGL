#include "Editor/Viewport/LightDebugOverlay.h"

#include "Engine/Scene/Components.h"

#include <imgui.h>

#include <glm/glm.hpp>

namespace Editor {
namespace {

bool projectWorldToScreen(
    const Engine::PerspectiveCamera& camera,
    const glm::vec3& world,
    float viewportWidth,
    float viewportHeight,
    ImVec2& outScreen) {
    const glm::vec4 clip =
        camera.projectionMatrix() * camera.viewMatrix() * glm::vec4(world, 1.0f);
    if (clip.w <= 0.0001f) {
        return false;
    }

    const glm::vec3 ndc = glm::vec3(clip) / clip.w;
    if (ndc.z < -1.0f || ndc.z > 1.0f) {
        return false;
    }

    outScreen.x = (ndc.x * 0.5f + 0.5f) * viewportWidth;
    outScreen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportHeight;
    return true;
}

void drawMarker(
    ImDrawList* drawList,
    const ImVec2& screen,
    ImU32 fill,
    ImU32 outline,
    const char* label,
    bool selected) {
    const float radius = selected ? 10.0f : 8.0f;
    drawList->AddCircleFilled(screen, radius, fill, 16);
    drawList->AddCircle(screen, radius, outline, 16, selected ? 2.5f : 1.5f);
    if (label != nullptr) {
        drawList->AddText(ImVec2(screen.x + radius + 4.0f, screen.y - 8.0f), outline, label);
    }
}

} // namespace

void drawLightDebugMarkers(
    Engine::Scene& scene,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    Engine::Registry& registry = scene.registry();
    const Engine::Entity selected = scene.selectedEntity();

    for (const auto& [entity, component] : registry.storage<Engine::PointLightComponent>()) {
        ImVec2 screen;
        if (!projectWorldToScreen(
                camera,
                component.light.position,
                viewportWidth,
                viewportHeight,
                screen)) {
            continue;
        }

        const ImU32 fill = ImGui::ColorConvertFloat4ToU32(ImVec4(
            component.light.color.r,
            component.light.color.g,
            component.light.color.b,
            0.9f));
        drawMarker(
            drawList,
            screen,
            fill,
            IM_COL32(255, 255, 255, 230),
            "Point",
            entity == selected);
    }

    for (const auto& [entity, component] : registry.storage<Engine::SpotLightComponent>()) {
        ImVec2 screen;
        if (!projectWorldToScreen(
                camera,
                component.light.position,
                viewportWidth,
                viewportHeight,
                screen)) {
            continue;
        }

        const ImU32 fill = ImGui::ColorConvertFloat4ToU32(ImVec4(
            component.light.color.r,
            component.light.color.g,
            component.light.color.b,
            0.9f));
        drawMarker(
            drawList,
            screen,
            fill,
            IM_COL32(180, 220, 255, 230),
            component.followCamera ? "Spot (cam)" : "Spot",
            entity == selected);
    }

    for (const auto& [entity, component] : registry.storage<Engine::DirectionalLightComponent>()) {
        glm::vec3 position(0.0f, 3.0f, 0.0f);
        if (registry.has<Engine::TransformComponent>(entity)) {
            position = registry.get<Engine::TransformComponent>(entity).transform.position();
        }

        ImVec2 screen;
        if (!projectWorldToScreen(camera, position, viewportWidth, viewportHeight, screen)) {
            continue;
        }

        // Draw a short direction indicator.
        const glm::vec3 tip = position + glm::normalize(component.light.direction) * 0.8f;
        ImVec2 tipScreen;
        if (projectWorldToScreen(camera, tip, viewportWidth, viewportHeight, tipScreen)) {
            drawList->AddLine(screen, tipScreen, IM_COL32(255, 240, 120, 220), 2.0f);
        }

        drawMarker(
            drawList,
            screen,
            IM_COL32(255, 220, 80, 220),
            IM_COL32(255, 255, 255, 230),
            "Dir",
            entity == selected);
        (void)component;
    }
}

} // namespace Editor
