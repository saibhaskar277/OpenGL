#include "Engine/Renderer/VertexBuffer.h"

#include <glad/glad.h>

#include <utility>

namespace Engine {

VertexBuffer::VertexBuffer(const void* data, std::size_t size) {
    glGenBuffers(1, &m_id);
    bind();
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
    release();
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : m_id(std::exchange(other.m_id, 0)) {
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
    if (this != &other) {
        release();
        m_id = std::exchange(other.m_id, 0);
    }
    return *this;
}

void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::release() {
    if (m_id != 0) {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }
}

} // namespace Engine
