#pragma once

#include <cstddef>

namespace Engine {

class IndexBuffer final {
public:
    IndexBuffer(const unsigned int* indices, std::size_t count);
    ~IndexBuffer();

    IndexBuffer(const IndexBuffer&) = delete;
    IndexBuffer& operator=(const IndexBuffer&) = delete;
    IndexBuffer(IndexBuffer&& other) noexcept;
    IndexBuffer& operator=(IndexBuffer&& other) noexcept;

    void bind() const;
    static void unbind();

    [[nodiscard]] int count() const;

private:
    void release();

    unsigned int m_id = 0;
    int m_count = 0;
};

} // namespace Engine
