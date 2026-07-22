#pragma once

#include <cstddef>

namespace Engine {

class VertexArray final {
public:
    VertexArray();
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    void bind() const;
    static void unbind();
    void addFloatAttribute(
        unsigned int index,
        int componentCount,
        int stride,
        std::size_t offset) const;

private:
    void release();

    unsigned int m_id = 0;
};

} // namespace Engine
