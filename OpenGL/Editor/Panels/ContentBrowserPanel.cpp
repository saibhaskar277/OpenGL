#include "Editor/Panels/ContentBrowserPanel.h"

#include "Editor/AssetPayloads.h"

#include <imgui.h>

#include <filesystem>

namespace Editor {
namespace {

const char* assetTypeLabel(Engine::AssetNodeType type) {
    switch (type) {
    case Engine::AssetNodeType::Folder:
        return "[D]";
    case Engine::AssetNodeType::Texture:
        return "[T]";
    case Engine::AssetNodeType::Model:
        return "[M]";
    case Engine::AssetNodeType::Scene:
        return "[S]";
    default:
        return "[?]";
    }
}

} // namespace

void ContentBrowserPanel::drawCreateMenus(
    Engine::AssetManager& assets,
    const std::string& folderRelativePath) {
    if (ImGui::MenuItem("Refresh")) {
        assets.refresh();
    }
    if (ImGui::MenuItem("Create Folder")) {
        m_contextFolderPath = folderRelativePath;
        m_showCreateFolderPopup = true;
    }
    if (ImGui::MenuItem("Create Scene")) {
        m_contextFolderPath = folderRelativePath.empty() ? "Scenes" : folderRelativePath;
        m_showCreateScenePopup = true;
    }
}

void ContentBrowserPanel::drawNode(
    const Engine::AssetNode& node,
    Engine::AssetManager& assets,
    std::string* openSceneRequest) {
    if (node.type == Engine::AssetNodeType::Folder) {
        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_DefaultOpen;
        if (m_selectedAssetPath == node.relativePath) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        const bool open = ImGui::TreeNodeEx(
            node.relativePath.empty() ? "Assets" : node.relativePath.c_str(),
            flags,
            "%s %s",
            assetTypeLabel(node.type),
            node.name.c_str());

        if (ImGui::IsItemClicked()) {
            m_selectedAssetPath = node.relativePath;
        }

        if (ImGui::BeginPopupContextItem()) {
            drawCreateMenus(assets, node.relativePath);
            ImGui::EndPopup();
        }

        if (open) {
            for (const Engine::AssetNode& child : node.children) {
                drawNode(child, assets, openSceneRequest);
            }
            ImGui::TreePop();
        }
        return;
    }

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen |
        ImGuiTreeNodeFlags_SpanAvailWidth;
    if (m_selectedAssetPath == node.relativePath) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    ImGui::TreeNodeEx(node.relativePath.c_str(), flags, "%s %s", assetTypeLabel(node.type), node.name.c_str());

    if (ImGui::IsItemClicked()) {
        m_selectedAssetPath = node.relativePath;
    }

    if (node.type == Engine::AssetNodeType::Scene &&
        ImGui::IsItemHovered() &&
        ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) &&
        openSceneRequest != nullptr) {
        *openSceneRequest = node.relativePath;
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        if (node.type == Engine::AssetNodeType::Model) {
            ImGui::SetDragDropPayload(
                AssetPayloadModel,
                node.relativePath.c_str(),
                node.relativePath.size() + 1);
            ImGui::Text("Model: %s", node.name.c_str());
        } else if (node.type == Engine::AssetNodeType::Texture) {
            ImGui::SetDragDropPayload(
                AssetPayloadTexture,
                node.relativePath.c_str(),
                node.relativePath.size() + 1);
            ImGui::Text("Texture: %s", node.name.c_str());
        } else if (node.type == Engine::AssetNodeType::Scene) {
            ImGui::SetDragDropPayload(
                AssetPayloadScene,
                node.relativePath.c_str(),
                node.relativePath.size() + 1);
            ImGui::Text("Scene: %s", node.name.c_str());
        }
        ImGui::EndDragDropSource();
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", node.relativePath.c_str());
    }
}

void ContentBrowserPanel::onImGuiRender(
    Engine::AssetManager& assets,
    std::string* openSceneRequest) {
    ImGui::Begin("Project");

    if (ImGui::Button("Refresh")) {
        assets.refresh();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Drag models/textures into Hierarchy or Inspector");

    ImGui::Separator();

    if (ImGui::BeginPopupContextWindow(
            "ProjectEmptyContext",
            ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        drawCreateMenus(assets, "");
        ImGui::EndPopup();
    }

    drawNode(assets.rootNode(), assets, openSceneRequest);

    if (m_showCreateFolderPopup) {
        ImGui::OpenPopup("Create Folder");
        m_showCreateFolderPopup = false;
    }
    if (ImGui::BeginPopupModal("Create Folder", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", m_newFolderName, sizeof(m_newFolderName));
        if (ImGui::Button("Create")) {
            assets.createFolder(m_contextFolderPath, m_newFolderName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_showCreateScenePopup) {
        ImGui::OpenPopup("Create Scene");
        m_showCreateScenePopup = false;
    }
    if (ImGui::BeginPopupModal("Create Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Name", m_newSceneName, sizeof(m_newSceneName));
        ImGui::TextDisabled("Saved under Assets/%s/", m_contextFolderPath.c_str());
        if (ImGui::Button("Create")) {
            std::string fileName = m_newSceneName;
            if (fileName.find('.') == std::string::npos) {
                fileName += ".json";
            }
            const std::string relative =
                m_contextFolderPath.empty()
                ? fileName
                : (m_contextFolderPath + "/" + fileName);
            if (assets.createEmptyScene(relative) && openSceneRequest != nullptr) {
                *openSceneRequest = relative;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

const std::string& ContentBrowserPanel::selectedAssetPath() const {
    return m_selectedAssetPath;
}

} // namespace Editor
