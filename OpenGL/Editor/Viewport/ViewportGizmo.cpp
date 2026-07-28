#include "Editor/Viewport/ViewportGizmo.h"

#include "Engine/Scene/Components.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

namespace Editor {
namespace {

void syncLightsFromTransform(Engine::Registry& registry, Engine::Entity entity) {
    if (!registry.has<Engine::TransformComponent>(entity)) {
        return;
    }
    const glm::vec3 position =
        registry.get<Engine::TransformComponent>(entity).transform.position();

    if (registry.has<Engine::PointLightComponent>(entity)) {
        registry.get<Engine::PointLightComponent>(entity).light.position = position;
    }
    if (registry.has<Engine::SpotLightComponent>(entity)) {
        auto& spot = registry.get<Engine::SpotLightComponent>(entity);
        if (!spot.followCamera) {
            spot.light.position = position;
        }
    }
}

void syncTransformFromLights(Engine::Registry& registry, Engine::Entity entity) {
    if (!registry.has<Engine::TransformComponent>(entity)) {
        return;
    }
    auto& transform = registry.get<Engine::TransformComponent>(entity).transform;

    if (registry.has<Engine::PointLightComponent>(entity)) {
        transform.setPosition(
            registry.get<Engine::PointLightComponent>(entity).light.position);
    } else if (registry.has<Engine::SpotLightComponent>(entity)) {
        const auto& spot = registry.get<Engine::SpotLightComponent>(entity);
        if (!spot.followCamera) {
            transform.setPosition(spot.light.position);
        }
    }
}

ImGuizmo::OPERATION toOperation(GizmoMode mode) {
    switch (mode) {
    case GizmoMode::Rotate:
        return ImGuizmo::ROTATE;
    case GizmoMode::Scale:
        return ImGuizmo::SCALE;
    case GizmoMode::Translate:
    default:
        return ImGuizmo::TRANSLATE;
    }
}

} // namespace

void ViewportGizmo::beginFrame() {
    ImGuizmo::BeginFrame();
}

bool ViewportGizmo::draw(
    Engine::Scene& scene,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight) {
    m_using = false;
    m_over = false;

    const Engine::Entity selected = scene.selectedEntity();
    if (selected == Engine::NullEntity || !scene.registry().alive(selected)) {
        return false;
    }
    if (!scene.registry().has<Engine::TransformComponent>(selected)) {
        return false;
    }

    syncTransformFromLights(scene.registry(), selected);

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0.0f, 0.0f, viewportWidth, viewportHeight);

    glm::mat4 view = camera.viewMatrix();
    glm::mat4 projection = camera.projectionMatrix();
    glm::mat4 model =
        scene.registry().get<Engine::TransformComponent>(selected).transform.matrix();

    // Lights only need translate (direction lights keep orientation via inspector).
    ImGuizmo::OPERATION operation = toOperation(m_mode);
    if (scene.registry().has<Engine::PointLightComponent>(selected) ||
        scene.registry().has<Engine::SpotLightComponent>(selected) ||
        scene.registry().has<Engine::DirectionalLightComponent>(selected) ||
        scene.registry().has<Engine::HemisphereLightComponent>(selected)) {
        operation = ImGuizmo::TRANSLATE;
    }

    ImGuizmo::Manipulate(
        glm::value_ptr(view),
        glm::value_ptr(projection),
        operation,
        ImGuizmo::WORLD,
        glm::value_ptr(model));

    m_using = ImGuizmo::IsUsing();
    m_over = ImGuizmo::IsOver();

    if (ImGuizmo::IsUsing()) {
        glm::vec3 translation;
        glm::vec3 rotation;
        glm::vec3 scale;
        ImGuizmo::DecomposeMatrixToComponents(
            glm::value_ptr(model),
            glm::value_ptr(translation),
            glm::value_ptr(rotation),
            glm::value_ptr(scale));

        Engine::Transform& transform =
            scene.registry().get<Engine::TransformComponent>(selected).transform;
        transform.setPosition(translation);
        transform.setRotationDegrees(rotation);
        transform.setScale(scale);
        syncLightsFromTransform(scene.registry(), selected);
    }

    return m_using || m_over;
}

void ViewportGizmo::setMode(GizmoMode mode) {
    m_mode = mode;
}

GizmoMode ViewportGizmo::mode() const {
    return m_mode;
}

bool ViewportGizmo::isUsing() const {
    return m_using;
}

bool ViewportGizmo::isOver() const {
    return m_over;
}

} // namespace Editor
