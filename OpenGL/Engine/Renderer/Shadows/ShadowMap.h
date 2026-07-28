#pragma once

namespace Engine {

class ShadowMap final {
public:
    ShadowMap(int width, int height);
    ~ShadowMap();

    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;
    ShadowMap(ShadowMap&& other) noexcept;
    ShadowMap& operator=(ShadowMap&& other) noexcept;

    void beginWrite() const;
    static void endWrite(int viewportWidth, int viewportHeight);
    void bindTexture(unsigned int textureUnit) const;

    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;

private:
    void release();

    unsigned int m_framebuffer = 0;
    unsigned int m_depthTexture = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace Engine
