#pragma once

#include "Engine/Renderer/RHI/Texture.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine {

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

class AssetManager final {
public:
    explicit AssetManager(std::filesystem::path root = "Assets");

    void setRoot(const std::filesystem::path& root);
    [[nodiscard]] const std::filesystem::path& root() const;

    void refresh();

    [[nodiscard]] const std::vector<TextureAssetInfo>& textures() const;
    [[nodiscard]] const std::vector<ModelAssetInfo>& models() const;

    std::shared_ptr<Texture> getTexture(
        const std::string& relativePath,
        TextureColorSpace colorSpace = TextureColorSpace::Srgb);

    void clearCache();

private:
    std::filesystem::path m_root;
    std::vector<TextureAssetInfo> m_textures;
    std::vector<ModelAssetInfo> m_models;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;
};

} // namespace Engine
