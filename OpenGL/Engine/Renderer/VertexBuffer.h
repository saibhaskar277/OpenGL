#pragma once

#include <cstddef>

namespace Engine {

class VertexBuffer final {
public:
    VertexBuffer(const void* data, std::size_t size);
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;
    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    void bind() const;
    static void unbind();

private:
    void release();

    unsigned int m_id = 0;
};

} // namespace Engine
