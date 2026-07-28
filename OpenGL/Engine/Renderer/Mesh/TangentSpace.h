#pragma once

#include "Engine/Renderer/Mesh/Mesh.h"

#include <vector>

namespace Engine {

void computeTangents(
    std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices);

void computeTangents(std::vector<Vertex>& vertices);

} // namespace Engine
