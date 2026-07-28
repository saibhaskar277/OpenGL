#include "Editor/Panels/ContentBrowserPanel.h"

#include <imgui.h>

namespace Editor {

void ContentBrowserPanel::onImGuiRender(Engine::AssetManager& assets) {
    ImGui::Begin("Content Browser");

    if (ImGui::Button("Refresh Assets")) {
        assets.refresh();
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Textures");
    for (const Engine::TextureAssetInfo& texture : assets.textures()) {
        const bool selected = m_selectedTextureId == texture.id;
        if (ImGui::Selectable(texture.displayName.c_str(), selected)) {
            m_selectedTextureId = texture.id;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", texture.id.c_str());
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Models");
    for (const Engine::ModelAssetInfo& model : assets.models()) {
        ImGui::BulletText("%s", model.displayName.c_str());
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", model.id.c_str());
        }
    }

    if (assets.textures().empty()) {
        ImGui::TextDisabled("Drop PNG/JPG files into Assets/Textures");
    }

    ImGui::End();
}

const std::string& ContentBrowserPanel::selectedTextureId() const {
    return m_selectedTextureId;
}

} // namespace Editor
