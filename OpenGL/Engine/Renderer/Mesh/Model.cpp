#include "Engine/Renderer/Mesh/Model.h"

#include "Engine/Renderer/Mesh/TangentSpace.h"
#include "Engine/Renderer/RHI/Texture.h"

#include <glm/glm.hpp>

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace Engine {
namespace {

struct ObjMaterialTemplate {
    Material material;
    std::string albedoPath;
    std::string normalPath;
    std::string specularPath;
    std::string emissivePath;
    std::string aoPath;
};

std::string trim(const std::string& value) {
    std::size_t begin = 0;
    while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin]))) {
        ++begin;
    }
    std::size_t end = value.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::filesystem::path resolveRelative(
    const std::filesystem::path& baseDirectory,
    const std::string& relative) {
    return (baseDirectory / relative).lexically_normal();
}

std::shared_ptr<Texture> tryLoadMap(
    const std::filesystem::path& path,
    TextureColorSpace colorSpace) {
    if (path.empty() || !std::filesystem::exists(path)) {
        return nullptr;
    }
    return std::make_shared<Texture>(Texture::fromFile(path, colorSpace));
}

std::unordered_map<std::string, ObjMaterialTemplate> loadMtl(
    const std::filesystem::path& mtlPath) {
    std::ifstream file(mtlPath);
    if (!file) {
        throw std::runtime_error("Failed to open MTL file: " + mtlPath.string());
    }

    const std::filesystem::path baseDirectory = mtlPath.parent_path();
    std::unordered_map<std::string, ObjMaterialTemplate> materials;
    ObjMaterialTemplate* current = nullptr;
    std::string line;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream stream(line);
        std::string tag;
        stream >> tag;

        if (tag == "newmtl") {
            std::string name;
            std::getline(stream, name);
            name = trim(name);
            current = &materials[name];
            current->material = Material{};
        } else if (current == nullptr) {
            continue;
        } else if (tag == "Kd") {
            stream >> current->material.albedo.r
                   >> current->material.albedo.g
                   >> current->material.albedo.b;
        } else if (tag == "Ks") {
            stream >> current->material.specularColor.r
                   >> current->material.specularColor.g
                   >> current->material.specularColor.b;
        } else if (tag == "Ke") {
            stream >> current->material.emissive.r
                   >> current->material.emissive.g
                   >> current->material.emissive.b;
        } else if (tag == "Ns") {
            stream >> current->material.shininess;
        } else if (tag == "map_Kd") {
            std::string path;
            std::getline(stream, path);
            current->albedoPath = trim(path);
        } else if (
            tag == "map_Bump" || tag == "bump" || tag == "norm" ||
            tag == "map_Kn" || tag == "map_normal") {
            std::string path;
            std::getline(stream, path);
            current->normalPath = trim(path);
        } else if (tag == "map_Ks") {
            std::string path;
            std::getline(stream, path);
            current->specularPath = trim(path);
        } else if (tag == "map_Ke") {
            std::string path;
            std::getline(stream, path);
            current->emissivePath = trim(path);
        } else if (tag == "map_Ka" || tag == "map_ao" || tag == "map_Occlusion") {
            std::string path;
            std::getline(stream, path);
            current->aoPath = trim(path);
        }
    }

    for (auto& [name, entry] : materials) {
        (void)name;
        if (!entry.albedoPath.empty()) {
            entry.material.albedoMap = tryLoadMap(
                resolveRelative(baseDirectory, entry.albedoPath),
                TextureColorSpace::Srgb);
        }
        if (!entry.normalPath.empty()) {
            entry.material.normalMap = tryLoadMap(
                resolveRelative(baseDirectory, entry.normalPath),
                TextureColorSpace::Linear);
        }
        if (!entry.specularPath.empty()) {
            entry.material.specularMap = tryLoadMap(
                resolveRelative(baseDirectory, entry.specularPath),
                TextureColorSpace::Linear);
        }
        if (!entry.emissivePath.empty()) {
            entry.material.emissiveMap = tryLoadMap(
                resolveRelative(baseDirectory, entry.emissivePath),
                TextureColorSpace::Srgb);
        }
        if (!entry.aoPath.empty()) {
            entry.material.aoMap = tryLoadMap(
                resolveRelative(baseDirectory, entry.aoPath),
                TextureColorSpace::Linear);
        }
    }

    return materials;
}

struct FaceCorner {
    int position = -1;
    int texCoord = -1;
    int normal = -1;
};

FaceCorner parseCorner(const std::string& token) {
    FaceCorner corner;
    std::size_t firstSlash = token.find('/');
    if (firstSlash == std::string::npos) {
        corner.position = std::stoi(token) - 1;
        return corner;
    }

    corner.position = std::stoi(token.substr(0, firstSlash)) - 1;
    std::size_t secondSlash = token.find('/', firstSlash + 1);
    if (secondSlash == std::string::npos) {
        const std::string uv = token.substr(firstSlash + 1);
        if (!uv.empty()) {
            corner.texCoord = std::stoi(uv) - 1;
        }
        return corner;
    }

    const std::string uv =
        token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
    if (!uv.empty()) {
        corner.texCoord = std::stoi(uv) - 1;
    }
    const std::string normal = token.substr(secondSlash + 1);
    if (!normal.empty()) {
        corner.normal = std::stoi(normal) - 1;
    }
    return corner;
}

struct MeshBuild {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::unordered_map<std::string, unsigned int> vertexCache;
};

unsigned int addCorner(
    MeshBuild& build,
    const FaceCorner& corner,
    const std::vector<glm::vec3>& positions,
    const std::vector<glm::vec2>& texCoords,
    const std::vector<glm::vec3>& normals) {
    const std::string key =
        std::to_string(corner.position) + "/" +
        std::to_string(corner.texCoord) + "/" +
        std::to_string(corner.normal);

    const auto found = build.vertexCache.find(key);
    if (found != build.vertexCache.end()) {
        return found->second;
    }

    if (corner.position < 0 ||
        static_cast<std::size_t>(corner.position) >= positions.size()) {
        throw std::runtime_error("OBJ face references an invalid position index");
    }

    Vertex vertex;
    vertex.position = positions[static_cast<std::size_t>(corner.position)];
    if (corner.texCoord >= 0 &&
        static_cast<std::size_t>(corner.texCoord) < texCoords.size()) {
        vertex.texCoord = texCoords[static_cast<std::size_t>(corner.texCoord)];
    }
    if (corner.normal >= 0 &&
        static_cast<std::size_t>(corner.normal) < normals.size()) {
        vertex.normal = normals[static_cast<std::size_t>(corner.normal)];
    } else {
        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    const unsigned int index = static_cast<unsigned int>(build.vertices.size());
    build.vertices.push_back(vertex);
    build.vertexCache.emplace(key, index);
    return index;
}

} // namespace

Model::Model(std::vector<ModelPart> parts)
    : m_parts(std::move(parts)) {
    if (m_parts.empty()) {
        throw std::invalid_argument("A model requires at least one mesh part");
    }
}

Model Model::loadObj(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open OBJ file: " + path.string());
    }

    const std::filesystem::path baseDirectory = path.parent_path();
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::unordered_map<std::string, ObjMaterialTemplate> materials;
    materials.emplace("default", ObjMaterialTemplate{});

    std::string activeMaterial = "default";
    std::unordered_map<std::string, MeshBuild> builds;
    builds.emplace(activeMaterial, MeshBuild{});

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream stream(line);
        std::string tag;
        stream >> tag;

        if (tag == "mtllib") {
            std::string mtlName;
            std::getline(stream, mtlName);
            mtlName = trim(mtlName);
            auto loaded = loadMtl(resolveRelative(baseDirectory, mtlName));
            for (auto& entry : loaded) {
                materials[entry.first] = std::move(entry.second);
            }
        } else if (tag == "usemtl") {
            std::string name;
            std::getline(stream, name);
            activeMaterial = trim(name);
            if (materials.find(activeMaterial) == materials.end()) {
                materials.emplace(activeMaterial, ObjMaterialTemplate{});
            }
            builds.emplace(activeMaterial, MeshBuild{});
        } else if (tag == "v") {
            glm::vec3 position;
            stream >> position.x >> position.y >> position.z;
            positions.push_back(position);
        } else if (tag == "vt") {
            glm::vec2 uv;
            stream >> uv.x >> uv.y;
            texCoords.push_back(uv);
        } else if (tag == "vn") {
            glm::vec3 normal;
            stream >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (tag == "f") {
            std::vector<std::string> tokens;
            std::string token;
            while (stream >> token) {
                tokens.push_back(token);
            }
            if (tokens.size() < 3) {
                continue;
            }

            MeshBuild& build = builds[activeMaterial];
            std::vector<unsigned int> faceIndices;
            faceIndices.reserve(tokens.size());
            for (const std::string& cornerToken : tokens) {
                faceIndices.push_back(addCorner(
                    build,
                    parseCorner(cornerToken),
                    positions,
                    texCoords,
                    normals));
            }

            for (std::size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                build.indices.push_back(faceIndices[0]);
                build.indices.push_back(faceIndices[i]);
                build.indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    std::vector<ModelPart> parts;
    for (auto& [materialName, build] : builds) {
        if (build.indices.empty()) {
            continue;
        }

        // Fill missing normals from triangle geometry.
        for (std::size_t i = 0; i + 2 < build.indices.size(); i += 3) {
            Vertex& v0 = build.vertices[build.indices[i]];
            Vertex& v1 = build.vertices[build.indices[i + 1]];
            Vertex& v2 = build.vertices[build.indices[i + 2]];
            if (glm::length(v0.normal) < 0.0001f ||
                glm::length(v1.normal) < 0.0001f ||
                glm::length(v2.normal) < 0.0001f) {
                const glm::vec3 faceNormal = glm::normalize(glm::cross(
                    v1.position - v0.position,
                    v2.position - v0.position));
                if (glm::length(v0.normal) < 0.0001f) {
                    v0.normal = faceNormal;
                }
                if (glm::length(v1.normal) < 0.0001f) {
                    v1.normal = faceNormal;
                }
                if (glm::length(v2.normal) < 0.0001f) {
                    v2.normal = faceNormal;
                }
            }
        }

        computeTangents(build.vertices, build.indices);

        Material material = materials[materialName].material;
        parts.push_back(ModelPart{
            Mesh(build.vertices, build.indices),
            std::move(material)});
    }

    if (parts.empty()) {
        throw std::runtime_error("OBJ contained no drawable faces: " + path.string());
    }

    return Model(std::move(parts));
}

const std::vector<ModelPart>& Model::parts() const {
    return m_parts;
}

std::vector<ModelPart>& Model::parts() {
    return m_parts;
}

} // namespace Engine
