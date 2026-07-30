#include "Editor/Panels/HierarchyPanel.h"

#include "Editor/AssetPayloads.h"
#include "Engine/Scene/Components.h"

#include <imgui.h>

#include <cstdint>
#include <filesystem>
#include <string>

namespace Editor {
namespace {

std::string modelDisplayName(const char* modelPath) {
    return std::filesystem::path(modelPath).stem().string();
}

bool acceptModelDrop(Engine::Scene& scene) {
    if (!ImGui::BeginDragDropTarget()) {
        return false;
    }

    bool changed = false;
    if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload(AssetPayloadModel)) {
        const char* modelPath = static_cast<const char*>(payload->Data);
        const Engine::Entity entity =
            scene.createModel(modelPath, modelDisplayName(modelPath));
        scene.setSelectedEntity(entity);
        changed = true;
    }

    if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload(AssetPayloadTexture)) {
        const char* texturePath = static_cast<const char*>(payload->Data);
        const Engine::Entity selected = scene.selectedEntity();
        if (selected != Engine::NullEntity &&
            scene.registry().alive(selected) &&
            scene.registry().has<Engine::MaterialComponent>(selected)) {
            scene.registry().get<Engine::MaterialComponent>(selected).albedoMapPath =
                texturePath;
            changed = true;
        }
    }

    ImGui::EndDragDropTarget();
    return changed;
}

} // namespace

bool HierarchyPanel::onImGuiRender(Engine::Scene& scene) {
    bool dirty = false;
    ImGui::Begin("Hierarchy");

    if (ImGui::BeginPopupContextWindow(
            "HierarchyContext",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("Add Cube")) {
            scene.setSelectedEntity(scene.createCube("Cube"));
            dirty = true;
        }
        if (ImGui::MenuItem("Add Physics Cube")) {
            scene.setSelectedEntity(scene.createPhysicsCube());
            dirty = true;
        }
        if (ImGui::BeginMenu("Add Light")) {
            if (ImGui::MenuItem("Directional Light")) {
                scene.setSelectedEntity(scene.createDirectionalLight());
                dirty = true;
            }
            if (ImGui::MenuItem("Point Light")) {
                scene.setSelectedEntity(scene.createPointLight());
                dirty = true;
            }
            if (ImGui::MenuItem("Spot Light")) {
                scene.setSelectedEntity(scene.createSpotLight());
                dirty = true;
            }
            if (ImGui::MenuItem("Hemisphere Light")) {
                scene.setSelectedEntity(scene.createHemisphereLight());
                dirty = true;
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

    // Full-window drop target for models/textures.
    ImGui::Dummy(ImGui::GetContentRegionAvail());
    if (acceptModelDrop(scene)) {
        dirty = true;
    }

    if (entityToDelete != Engine::NullEntity) {
        scene.destroyEntity(entityToDelete);
        dirty = true;
    }

    ImGui::End();
    return dirty;
}

} // namespace Editor
