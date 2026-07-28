#pragma once

#include "Engine/Renderer/RHI/Material.h"
#include "Engine/Renderer/Mesh/Mesh.h"

#include <filesystem>
#include <vector>

namespace Engine {

struct ModelPart {
    Mesh mesh;
    Material material;
};

class Model final {
public:
    explicit Model(std::vector<ModelPart> parts);

    // Loads Wavefront OBJ (+ optional MTL maps via stb_image).
    static Model loadObj(const std::filesystem::path& path);

    [[nodiscard]] const std::vector<ModelPart>& parts() const;
    [[nodiscard]] std::vector<ModelPart>& parts();

private:
    std::vector<ModelPart> m_parts;
};

} // namespace Engine
