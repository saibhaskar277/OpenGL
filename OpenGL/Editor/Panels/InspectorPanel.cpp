#include "Editor/Panels/InspectorPanel.h"

#include "Editor/AssetPayloads.h"
#include "Engine/Scene/Components.h"

#include <imgui.h>

#include <cstdio>

namespace Editor {

bool InspectorPanel::drawTextureSlot(
    const char* label,
    std::string& path,
    Engine::AssetManager& assets) {
    bool changed = false;
    const std::string preview = path.empty() ? "(None) — drop texture here" : path;
    if (ImGui::BeginCombo(label, preview.c_str())) {
        if (ImGui::Selectable("(None)", path.empty())) {
            path.clear();
            changed = true;
        }
        for (const Engine::TextureAssetInfo& texture : assets.textures()) {
            const bool selected = path == texture.id;
            if (ImGui::Selectable(texture.displayName.c_str(), selected)) {
                path = texture.id;
                changed = true;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload(AssetPayloadTexture)) {
            path = static_cast<const char*>(payload->Data);
            changed = true;
        }
        ImGui::EndDragDropTarget();
    }

    return changed;
}

bool InspectorPanel::onImGuiRender(Engine::Scene& scene, Engine::AssetManager& assets) {
    bool dirty = false;
    ImGui::Begin("Inspector");

    const Engine::Entity selected = scene.selectedEntity();
    if (selected == Engine::NullEntity || !scene.registry().alive(selected)) {
        ImGui::TextUnformatted("No entity selected");
        ImGui::TextDisabled("Drag a model from Project into Hierarchy to create one.");
        ImGui::End();
        return false;
    }

    Engine::Registry& registry = scene.registry();

    if (registry.has<Engine::TagComponent>(selected)) {
        auto& tag = registry.get<Engine::TagComponent>(selected);
        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), "%s", tag.name.c_str());
        if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
            tag.name = buffer;
            dirty = true;
        }
    }

    if (registry.has<Engine::TransformComponent>(selected)) {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            Engine::Transform& transform =
                registry.get<Engine::TransformComponent>(selected).transform;
            glm::vec3 position = transform.position();
            glm::vec3 rotation = transform.rotationDegrees();
            glm::vec3 scale = transform.scale();
            if (ImGui::DragFloat3("Position", &position.x, 0.05f)) {
                transform.setPosition(position);
                if (registry.has<Engine::PointLightComponent>(selected)) {
                    registry.get<Engine::PointLightComponent>(selected).light.position =
                        position;
                }
                if (registry.has<Engine::SpotLightComponent>(selected)) {
                    auto& spot = registry.get<Engine::SpotLightComponent>(selected);
                    if (!spot.followCamera) {
                        spot.light.position = position;
                    }
                }
                dirty = true;
            }
            if (ImGui::DragFloat3("Rotation", &rotation.x, 0.5f)) {
                transform.setRotationDegrees(rotation);
                dirty = true;
            }
            if (ImGui::DragFloat3("Scale", &scale.x, 0.05f)) {
                transform.setScale(scale);
                dirty = true;
            }
        }
    }

    if (registry.has<Engine::MeshRendererComponent>(selected)) {
        if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& mesh = registry.get<Engine::MeshRendererComponent>(selected);
            int meshType = static_cast<int>(mesh.meshType);
            const char* meshTypes[] = {"Cube", "Model"};
            if (ImGui::Combo("Mesh Type", &meshType, meshTypes, 2)) {
                mesh.meshType = static_cast<Engine::MeshType>(meshType);
                dirty = true;
            }
            if (mesh.meshType == Engine::MeshType::Model) {
                ImGui::TextWrapped(
                    "Imported OBJ uses MTL textures per mesh part automatically. "
                    "Assign maps below only to override those MTL textures.");
                ImGui::Text("Model Path: %s",
                    mesh.modelPath.empty() ? "(None)" : mesh.modelPath.c_str());
                const std::string preview =
                    mesh.modelPath.empty() ? "(None)" : mesh.modelPath;
                if (ImGui::BeginCombo("Model", preview.c_str())) {
                    for (const Engine::ModelAssetInfo& model : assets.models()) {
                        const bool selectedModel = mesh.modelPath == model.id;
                        if (ImGui::Selectable(model.displayName.c_str(), selectedModel)) {
                            mesh.modelPath = model.id;
                            dirty = true;
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload =
                            ImGui::AcceptDragDropPayload(AssetPayloadModel)) {
                        mesh.modelPath = static_cast<const char*>(payload->Data);
                        mesh.meshType = Engine::MeshType::Model;
                        dirty = true;
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            dirty |= ImGui::Checkbox("Cast Shadows", &mesh.castShadows);
            dirty |= ImGui::Checkbox("Visible", &mesh.visible);
        }
    } else if (ImGui::Button("Add Mesh Renderer")) {
        registry.add<Engine::MeshRendererComponent>(selected);
        if (!registry.has<Engine::MaterialComponent>(selected)) {
            registry.add<Engine::MaterialComponent>(selected);
        }
        dirty = true;
    }

    if (registry.has<Engine::MaterialComponent>(selected)) {
        if (ImGui::CollapsingHeader("Material / Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& material = registry.get<Engine::MaterialComponent>(selected);
            ImGui::TextDisabled("Drop textures from Project onto map slots below.");
            dirty |= ImGui::ColorEdit3("Albedo", &material.albedo.x);
            dirty |= ImGui::ColorEdit3("Specular", &material.specularColor.x);
            dirty |= ImGui::DragFloat("Shininess", &material.shininess, 0.5f, 1.0f, 256.0f);
            dirty |= ImGui::ColorEdit3("Emissive", &material.emissive.x);
            dirty |= ImGui::DragFloat("AO", &material.ambientOcclusion, 0.01f, 0.0f, 1.0f);
            dirty |= drawTextureSlot("Albedo Map", material.albedoMapPath, assets);
            dirty |= drawTextureSlot("Normal Map", material.normalMapPath, assets);
            dirty |= drawTextureSlot("Specular Map", material.specularMapPath, assets);
            dirty |= drawTextureSlot("Emissive Map", material.emissiveMapPath, assets);
            dirty |= drawTextureSlot("AO Map", material.aoMapPath, assets);
        }
    }

    if (registry.has<Engine::DirectionalLightComponent>(selected)) {
        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& light = registry.get<Engine::DirectionalLightComponent>(selected).light;
            dirty |= ImGui::DragFloat3("Direction", &light.direction.x, 0.01f);
            dirty |= ImGui::ColorEdit3("Color", &light.color.x);
            dirty |= ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 20.0f);
            dirty |= ImGui::Checkbox("Enabled", &light.enabled);
        }
    }

    if (registry.has<Engine::PointLightComponent>(selected)) {
        if (ImGui::CollapsingHeader("Point Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& light = registry.get<Engine::PointLightComponent>(selected).light;
            if (ImGui::DragFloat3("Position", &light.position.x, 0.05f)) {
                if (registry.has<Engine::TransformComponent>(selected)) {
                    registry.get<Engine::TransformComponent>(selected)
                        .transform.setPosition(light.position);
                }
                dirty = true;
            }
            dirty |= ImGui::ColorEdit3("Color", &light.color.x);
            dirty |= ImGui::DragFloat("Intensity", &light.intensity, 0.05f, 0.0f, 50.0f);
            dirty |= ImGui::DragFloat("Constant", &light.constantAttenuation, 0.01f, 0.0f, 10.0f);
            dirty |= ImGui::DragFloat("Linear", &light.linearAttenuation, 0.001f, 0.0f, 5.0f);
            dirty |= ImGui::DragFloat(
                "Quadratic",
                &light.quadraticAttenuation,
                0.001f,
                0.0f,
                5.0f);
        }
    }

    if (registry.has<Engine::SpotLightComponent>(selected)) {
        if (ImGui::CollapsingHeader("Spot Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& component = registry.get<Engine::SpotLightComponent>(selected);
            auto& light = component.light;
            dirty |= ImGui::Checkbox("Follow Camera", &component.followCamera);
            if (ImGui::DragFloat3("Position", &light.position.x, 0.05f)) {
                if (registry.has<Engine::TransformComponent>(selected)) {
                    registry.get<Engine::TransformComponent>(selected)
                        .transform.setPosition(light.position);
                }
                dirty = true;
            }
            dirty |= ImGui::DragFloat3("Direction", &light.direction.x, 0.01f);
            dirty |= ImGui::ColorEdit3("Color", &light.color.x);
            dirty |= ImGui::DragFloat("Intensity", &light.intensity, 0.05f, 0.0f, 50.0f);
            dirty |= ImGui::DragFloat("Inner Cutoff", &light.innerCutoffDegrees, 0.1f, 0.0f, 90.0f);
            dirty |= ImGui::DragFloat("Outer Cutoff", &light.outerCutoffDegrees, 0.1f, 0.0f, 90.0f);
            dirty |= ImGui::Checkbox("Enabled", &light.enabled);
        }
    }

    if (registry.has<Engine::HemisphereLightComponent>(selected)) {
        if (ImGui::CollapsingHeader("Hemisphere Light", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto& light = registry.get<Engine::HemisphereLightComponent>(selected).light;
            ImGui::TextWrapped(
                "Ambient fill: upward normals get sky color, downward get ground.");
            dirty |= ImGui::ColorEdit3("Sky", &light.skyColor.x);
            dirty |= ImGui::ColorEdit3("Ground", &light.groundColor.x);
            dirty |= ImGui::DragFloat("Intensity", &light.intensity, 0.01f, 0.0f, 5.0f);
            dirty |= ImGui::DragFloat3("Up Direction", &light.upDirection.x, 0.01f);
            dirty |= ImGui::Checkbox("Enabled", &light.enabled);
        }
    }

    ImGui::End();
    return dirty;
}

} // namespace Editor
