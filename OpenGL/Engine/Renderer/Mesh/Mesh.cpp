#include "Engine/Renderer/Mesh/Mesh.h"

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

    configureAttributes();
}

Mesh::Mesh(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices)
    : m_vertexArray(),
      m_vertexBuffer(vertices.data(), vertices.size() * sizeof(Vertex)),
      m_indexBuffer(std::make_unique<IndexBuffer>(indices.data(), indices.size())),
      m_vertexCount(static_cast<int>(vertices.size())) {
    if (vertices.empty()) {
        throw std::invalid_argument("A mesh requires at least one vertex");
    }
    if (indices.empty()) {
        throw std::invalid_argument("An indexed mesh requires at least one index");
    }

    configureAttributes();
    m_indexBuffer->bind();
}

void Mesh::configureAttributes() {
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
    m_vertexArray.addFloatAttribute(
        2,
        3,
        static_cast<int>(sizeof(Vertex)),
        offsetof(Vertex, tangent));
    m_vertexArray.addFloatAttribute(
        3,
        2,
        static_cast<int>(sizeof(Vertex)),
        offsetof(Vertex, texCoord));
}

const VertexArray& Mesh::vertexArray() const {
    return m_vertexArray;
}

int Mesh::vertexCount() const {
    return m_vertexCount;
}

bool Mesh::indexed() const {
    return m_indexBuffer != nullptr;
}

int Mesh::indexCount() const {
    return m_indexBuffer != nullptr ? m_indexBuffer->count() : 0;
}

} // namespace Engine
