#include "Editor/Panels/HierarchyPanel.h"

#include "Engine/Scene/Components.h"

#include <imgui.h>

#include <cstdint>
#include <string>

namespace Editor {

void HierarchyPanel::onImGuiRender(Engine::Scene& scene) {
    ImGui::Begin("Hierarchy");

    if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight |
                                                           ImGuiPopupFlags_NoOpenOverItems)) {
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
        ImGui::EndPopup();
    }

    Engine::Registry& registry = scene.registry();
    Engine::Entity entityToDelete = Engine::NullEntity;

    for (Engine::Entity entity : registry.entities()) {
        const std::string& name =
            registry.has<Engine::TagComponent>(entity)
            ? registry.get<Engine::TagComponent>(entity).name
            : "Entity";

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_Leaf;
        if (scene.selectedEntity() == entity) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        const bool opened = ImGui::TreeNodeEx(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity)),
            flags,
            "%s",
            name.c_str());

        if (ImGui::IsItemClicked()) {
            scene.setSelectedEntity(entity);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                entityToDelete = entity;
            }
            ImGui::EndPopup();
        }

        if (opened) {
            ImGui::TreePop();
        }
    }

    if (entityToDelete != Engine::NullEntity) {
        scene.destroyEntity(entityToDelete);
    }

    ImGui::End();
}

} // namespace Editor
