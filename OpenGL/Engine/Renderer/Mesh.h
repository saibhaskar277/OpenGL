#pragma once

#include "Engine/Renderer/IndexBuffer.h"
#include "Engine/Renderer/VertexArray.h"
#include "Engine/Renderer/VertexBuffer.h"

#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace Engine {

struct Vertex {
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec3 tangent{1.0f, 0.0f, 0.0f};
    glm::vec2 texCoord{0.0f};
};

class Mesh final {
public:
    explicit Mesh(const std::vector<Vertex>& vertices);
    Mesh(
        const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices);

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    [[nodiscard]] const VertexArray& vertexArray() const;
    [[nodiscard]] int vertexCount() const;
    [[nodiscard]] bool indexed() const;
    [[nodiscard]] int indexCount() const;

private:
    void configureAttributes();

    VertexArray m_vertexArray;
    VertexBuffer m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;
    int m_vertexCount = 0;
};

} // namespace Engine
