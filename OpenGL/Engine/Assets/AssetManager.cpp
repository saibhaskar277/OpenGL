#include "Engine/Assets/AssetManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>

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

bool isUnderScenesFolder(const std::string& relativePath) {
    return relativePath == "Scenes" ||
        relativePath.rfind("Scenes/", 0) == 0 ||
        relativePath.find("/Scenes/") != std::string::npos;
}

AssetNodeType classifyFile(const std::filesystem::path& path, const std::string& relativePath) {
    if (hasExtension(path, {".png", ".jpg", ".jpeg", ".tga", ".bmp"})) {
        return AssetNodeType::Texture;
    }
    if (hasExtension(path, {".obj"})) {
        return AssetNodeType::Model;
    }
    if (hasExtension(path, {".json"}) && isUnderScenesFolder(relativePath)) {
        return AssetNodeType::Scene;
    }
    return AssetNodeType::Other;
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

AssetNode AssetManager::buildNode(const std::filesystem::path& absolutePath) const {
    AssetNode node;
    node.name = absolutePath == m_root ? "Assets" : absolutePath.filename().string();
    node.relativePath =
        absolutePath == m_root ? std::string{} : relativeUnix(m_root, absolutePath);

    if (std::filesystem::is_directory(absolutePath)) {
        node.type = AssetNodeType::Folder;
        std::vector<std::filesystem::directory_entry> entries;
        for (const auto& entry : std::filesystem::directory_iterator(absolutePath)) {
            entries.push_back(entry);
        }
        std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
            const bool aDir = a.is_directory();
            const bool bDir = b.is_directory();
            if (aDir != bDir) {
                return aDir && !bDir;
            }
            return a.path().filename().string() < b.path().filename().string();
        });

        for (const auto& entry : entries) {
            const auto& path = entry.path();
            const std::string name = path.filename().string();
            if (!name.empty() && name[0] == '.') {
                continue;
            }
            if (entry.is_directory()) {
                node.children.push_back(buildNode(path));
                continue;
            }
            if (!entry.is_regular_file()) {
                continue;
            }

            const std::string relative = relativeUnix(m_root, path);
            const AssetNodeType type = classifyFile(path, relative);
            if (type == AssetNodeType::Other) {
                continue;
            }

            AssetNode child;
            child.type = type;
            child.name = name;
            child.relativePath = relative;
            node.children.push_back(std::move(child));
        }
        return node;
    }

    node.type = classifyFile(absolutePath, node.relativePath);
    return node;
}

void AssetManager::collectFlatLists(const AssetNode& node) {
    if (node.type == AssetNodeType::Texture) {
        TextureAssetInfo info;
        info.id = node.relativePath;
        info.path = m_root / node.relativePath;
        info.displayName = node.name;
        m_textures.push_back(std::move(info));
    } else if (node.type == AssetNodeType::Model) {
        ModelAssetInfo info;
        info.id = node.relativePath;
        info.path = m_root / node.relativePath;
        info.displayName = node.name;
        m_models.push_back(std::move(info));
    } else if (node.type == AssetNodeType::Scene) {
        SceneAssetInfo info;
        info.id = node.relativePath;
        info.path = m_root / node.relativePath;
        info.displayName = node.name;
        m_scenes.push_back(std::move(info));
    }

    for (const AssetNode& child : node.children) {
        collectFlatLists(child);
    }
}

void AssetManager::refresh() {
    m_textures.clear();
    m_models.clear();
    m_scenes.clear();
    m_rootNode = AssetNode{};

    if (!std::filesystem::exists(m_root)) {
        std::filesystem::create_directories(m_root);
    }

    m_rootNode = buildNode(m_root);
    collectFlatLists(m_rootNode);

    std::sort(m_textures.begin(), m_textures.end(), [](const auto& a, const auto& b) {
        return a.displayName < b.displayName;
    });
    std::sort(m_models.begin(), m_models.end(), [](const auto& a, const auto& b) {
        return a.displayName < b.displayName;
    });
    std::sort(m_scenes.begin(), m_scenes.end(), [](const auto& a, const auto& b) {
        return a.displayName < b.displayName;
    });
}

const AssetNode& AssetManager::rootNode() const {
    return m_rootNode;
}

const std::vector<TextureAssetInfo>& AssetManager::textures() const {
    return m_textures;
}

const std::vector<ModelAssetInfo>& AssetManager::models() const {
    return m_models;
}

const std::vector<SceneAssetInfo>& AssetManager::scenes() const {
    return m_scenes;
}

bool AssetManager::createFolder(const std::string& relativeParent, const std::string& folderName) {
    if (folderName.empty()) {
        return false;
    }
    const std::filesystem::path path =
        relativeParent.empty() ? m_root / folderName : m_root / relativeParent / folderName;
    std::error_code error;
    const bool created = std::filesystem::create_directories(path, error);
    if (!error) {
        refresh();
    }
    return created || std::filesystem::is_directory(path);
}

bool AssetManager::createEmptyScene(const std::string& relativePath) {
    if (relativePath.empty()) {
        return false;
    }
    const std::filesystem::path fullPath = m_root / relativePath;
    std::error_code error;
    std::filesystem::create_directories(fullPath.parent_path(), error);
    if (error) {
        return false;
    }

    std::ofstream file(fullPath);
    if (!file) {
        return false;
    }
    file << "{\n  \"version\": 1,\n  \"entities\": []\n}\n";
    refresh();
    return true;
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
