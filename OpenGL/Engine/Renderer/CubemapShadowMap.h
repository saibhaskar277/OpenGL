#pragma once

namespace Engine {

class CubemapShadowMap final {
public:
    explicit CubemapShadowMap(int resolution);
    ~CubemapShadowMap();

    CubemapShadowMap(const CubemapShadowMap&) = delete;
    CubemapShadowMap& operator=(const CubemapShadowMap&) = delete;
    CubemapShadowMap(CubemapShadowMap&& other) noexcept;
    CubemapShadowMap& operator=(CubemapShadowMap&& other) noexcept;

    void beginFace(int face) const;
    static void endWrite(int viewportWidth, int viewportHeight);
    void bindTexture(unsigned int textureUnit) const;

    [[nodiscard]] int resolution() const;

private:
    void release();

    unsigned int m_framebuffer = 0;
    unsigned int m_depthCubemap = 0;
    int m_resolution = 0;
};

} // namespace Engine
