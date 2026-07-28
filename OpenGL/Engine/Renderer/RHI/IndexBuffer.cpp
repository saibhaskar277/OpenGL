#include "Engine/Renderer/RHI/IndexBuffer.h"

#include <glad/glad.h>

#include <stdexcept>
#include <utility>

namespace Engine {

IndexBuffer::IndexBuffer(const unsigned int* indices, std::size_t count)
    : m_count(static_cast<int>(count)) {
    if (indices == nullptr || count == 0) {
        throw std::invalid_argument("Index buffer requires at least one index");
    }

    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(count * sizeof(unsigned int)),
        indices,
        GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer() {
    release();
}

IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
    : m_id(std::exchange(other.m_id, 0)),
      m_count(std::exchange(other.m_count, 0)) {
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept {
    if (this != &other) {
        release();
        m_id = std::exchange(other.m_id, 0);
        m_count = std::exchange(other.m_count, 0);
    }
    return *this;
}

void IndexBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void IndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

int IndexBuffer::count() const {
    return m_count;
}

void IndexBuffer::release() {
    if (m_id != 0) {
        glDeleteBuffers(1, &m_id);
        m_id = 0;
    }
    m_count = 0;
}

} // namespace Engine
