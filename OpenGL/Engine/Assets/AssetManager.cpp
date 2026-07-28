#include "Engine/Assets/AssetManager.h"

#include <algorithm>
#include <cctype>

namespace Engine {
namespace {

bool hasExtension(const std::filesystem::path& path, const std::vector<std::string>& extensions) {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

std::string relativeUnix(const std::filesystem::path& root, const std::filesystem::path& path) {
    return std::filesystem::relative(path, root).generic_string();
}

} // namespace

AssetManager::AssetManager(std::filesystem::path root)
    : m_root(std::move(root)) {
}

void AssetManager::setRoot(const std::filesystem::path& root) {
    m_root = root;
}

const std::filesystem::path& AssetManager::root() const {
    return m_root;
}

void AssetManager::refresh() {
    m_textures.clear();
    m_models.clear();

    const auto texturesDir = m_root / "Textures";
    if (std::filesystem::exists(texturesDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(texturesDir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            if (!hasExtension(entry.path(), {".png", ".jpg", ".jpeg", ".tga", ".bmp"})) {
                continue;
            }
            TextureAssetInfo info;
            info.path = entry.path();
            info.id = relativeUnix(m_root, entry.path());
            info.displayName = entry.path().filename().string();
            m_textures.push_back(std::move(info));
        }
    }

    const auto modelsDir = m_root / "Models";
    if (std::filesystem::exists(modelsDir)) {
        for (const auto& entry : std::filesystem::directory_iterator(modelsDir)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            if (!hasExtension(entry.path(), {".obj"})) {
                continue;
            }
            ModelAssetInfo info;
            info.path = entry.path();
            info.id = relativeUnix(m_root, entry.path());
            info.displayName = entry.path().filename().string();
            m_models.push_back(std::move(info));
        }
    }

    std::sort(m_textures.begin(), m_textures.end(), [](const auto& a, const auto& b) {
        return a.displayName < b.displayName;
    });
    std::sort(m_models.begin(), m_models.end(), [](const auto& a, const auto& b) {
        return a.displayName < b.displayName;
    });
}

const std::vector<TextureAssetInfo>& AssetManager::textures() const {
    return m_textures;
}

const std::vector<ModelAssetInfo>& AssetManager::models() const {
    return m_models;
}

std::shared_ptr<Texture> AssetManager::getTexture(
    const std::string& relativePath,
    TextureColorSpace colorSpace) {
    if (relativePath.empty()) {
        return nullptr;
    }

    const std::string cacheKey =
        relativePath + "|" +
        (colorSpace == TextureColorSpace::Srgb ? "srgb" : "linear");
    const auto found = m_textureCache.find(cacheKey);
    if (found != m_textureCache.end()) {
        return found->second;
    }

    const std::filesystem::path fullPath = m_root / relativePath;
    if (!std::filesystem::exists(fullPath)) {
        return nullptr;
    }

    auto texture = std::make_shared<Texture>(
        Texture::fromFile(fullPath, colorSpace));
    m_textureCache.emplace(cacheKey, texture);
    return texture;
}

void AssetManager::clearCache() {
    m_textureCache.clear();
}

} // namespace Engine
