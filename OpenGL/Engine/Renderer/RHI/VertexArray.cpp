#include "Engine/Renderer/RHI/VertexArray.h"

#include <glad/glad.h>

#include <utility>

namespace Engine {

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_id);
}

VertexArray::~VertexArray() {
    release();
}

VertexArray::VertexArray(VertexArray&& other) noexcept
    : m_id(std::exchange(other.m_id, 0)) {
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
    if (this != &other) {
        release();
        m_id = std::exchange(other.m_id, 0);
    }
    return *this;
}

void VertexArray::bind() const {
    glBindVertexArray(m_id);
}

void VertexArray::unbind() {
    glBindVertexArray(0);
}

void VertexArray::addFloatAttribute(
    unsigned int index,
    int componentCount,
    int stride,
    std::size_t offset) const {
    bind();
    glVertexAttribPointer(
        index,
        componentCount,
        GL_FLOAT,
        GL_FALSE,
        stride,
        reinterpret_cast<const void*>(offset));
    glEnableVertexAttribArray(index);
}

void VertexArray::release() {
    if (m_id != 0) {
        glDeleteVertexArrays(1, &m_id);
        m_id = 0;
    }
}

} // namespace Engine
