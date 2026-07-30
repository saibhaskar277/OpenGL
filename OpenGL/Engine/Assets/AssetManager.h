#pragma once

#include "Engine/Renderer/RHI/Texture.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

enum class AssetNodeType {
    Folder = 0,
    Texture = 1,
    Model = 2,
    Scene = 3,
    Other = 4
};

struct AssetNode {
    AssetNodeType type = AssetNodeType::Folder;
    std::string name;
    std::string relativePath;
    std::vector<AssetNode> children;
};

struct TextureAssetInfo {
    std::string id;
    std::filesystem::path path;
    std::string displayName;
};

struct ModelAssetInfo {
    std::string id;
    std::filesystem::path path;
    std::string displayName;
};

struct SceneAssetInfo {
    std::string id;
    std::filesystem::path path;
    std::string displayName;
};

class AssetManager final {
public:
    explicit AssetManager(std::filesystem::path root = "Assets");

    void setRoot(const std::filesystem::path& root);
    [[nodiscard]] const std::filesystem::path& root() const;

    void refresh();

    [[nodiscard]] const AssetNode& rootNode() const;
    [[nodiscard]] const std::vector<TextureAssetInfo>& textures() const;
    [[nodiscard]] const std::vector<ModelAssetInfo>& models() const;
    [[nodiscard]] const std::vector<SceneAssetInfo>& scenes() const;

    bool createFolder(const std::string& relativeParent, const std::string& folderName);
    bool createEmptyScene(const std::string& relativePath);

    std::shared_ptr<Texture> getTexture(
        const std::string& relativePath,
        TextureColorSpace colorSpace = TextureColorSpace::Srgb);

    void clearCache();

private:
    AssetNode buildNode(const std::filesystem::path& absolutePath) const;
    void collectFlatLists(const AssetNode& node);

    std::filesystem::path m_root;
    AssetNode m_rootNode;
    std::vector<TextureAssetInfo> m_textures;
    std::vector<ModelAssetInfo> m_models;
    std::vector<SceneAssetInfo> m_scenes;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
};

} // namespace Engine
