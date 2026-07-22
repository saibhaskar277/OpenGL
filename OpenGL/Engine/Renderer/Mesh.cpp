#include "Engine/Renderer/Mesh.h"

#include <cstddef>
#include <stdexcept>

namespace Engine {

Mesh::Mesh(const std::vector<Vertex>& vertices)
    : m_vertexArray(),
      m_vertexBuffer(vertices.data(), vertices.size() * sizeof(Vertex)),
      m_vertexCount(static_cast<int>(vertices.size())) {
    if (vertices.empty()) {
        throw std::invalid_argument("A mesh requires at least one vertex");
    }

    m_vertexArray.bind();
    m_vertexBuffer.bind();
    m_vertexArray.addFloatAttribute(
        0,
        3,
        static_cast<int>(sizeof(Vertex)),
        offsetof(Vertex, position));
    m_vertexArray.addFloatAttribute(
        1,
        3,
        static_cast<int>(sizeof(Vertex)),
        offsetof(Vertex, normal));
}

const VertexArray& Mesh::vertexArray() const {
    return m_vertexArray;
}

int Mesh::vertexCount() const {
    return m_vertexCount;
}

} // namespace Engine
