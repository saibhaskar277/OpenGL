#pragma once

#include "Engine/Assets/AssetManager.h"

#include <string>

namespace Editor {

class ContentBrowserPanel final {
public:
    // If a scene asset is double-clicked, openSceneRequest is set to its Assets-relative path.
    void onImGuiRender(
        Engine::AssetManager& assets,
        std::string* openSceneRequest);

    [[nodiscard]] const std::string& selectedAssetPath() const;

private:
    void drawNode(
        const Engine::AssetNode& node,
        Engine::AssetManager& assets,
        std::string* openSceneRequest);
    void drawCreateMenus(Engine::AssetManager& assets, const std::string& folderRelativePath);

    std::string m_selectedAssetPath;
    std::string m_contextFolderPath;
    char m_newFolderName[128] = "NewFolder";
    char m_newSceneName[128] = "NewScene";
    bool m_showCreateFolderPopup = false;
    bool m_showCreateScenePopup = false;
};

} // namespace Editor
