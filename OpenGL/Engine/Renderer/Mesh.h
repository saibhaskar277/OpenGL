#pragma once

#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/VertexBuffer.h"

#include <glm/glm.hpp>

#include <vector>

namespace Engine {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class Mesh final {
public:
    explicit Mesh(const std::vector<Vertex>& vertices);

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    [[nodiscard]] const VertexArray& vertexArray() const;
    [[nodiscard]] int vertexCount() const;

private:
    VertexArray m_vertexArray;
    VertexBuffer m_vertexBuffer;
    int m_vertexCount = 0;
};

} // namespace Engine
