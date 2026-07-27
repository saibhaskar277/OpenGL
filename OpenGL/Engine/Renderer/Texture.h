#pragma once

#include <filesystem>
#include <memory>
#include <vector>

namespace Engine {

enum class TextureColorSpace {
    Linear,
    Srgb
};

enum class TextureWrap {
    Repeat,
    ClampToEdge,
    MirroredRepeat
};

enum class TextureFilter {
    Linear,
    Nearest,
    LinearMipmap
};

class Texture final {
public:
    Texture(
        int width,
        int height,
        int channels,
        const unsigned char* pixels,
        TextureColorSpace colorSpace = TextureColorSpace::Linear,
        TextureWrap wrap = TextureWrap::Repeat,
        TextureFilter filter = TextureFilter::LinearMipmap);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    static Texture fromFile(
        const std::filesystem::path& path,
        TextureColorSpace colorSpace = TextureColorSpace::Srgb,
        TextureWrap wrap = TextureWrap::Repeat,
        TextureFilter filter = TextureFilter::LinearMipmap);

    static std::shared_ptr<Texture> createSolid(
        unsigned char red,
        unsigned char green,
        unsigned char blue,
        unsigned char alpha = 255,
        TextureColorSpace colorSpace = TextureColorSpace::Srgb);

    static std::shared_ptr<Texture> createCheckerboard(
        int resolution = 256,
        int checkerSize = 32,
        unsigned char colorA = 220,
        unsigned char colorB = 40);

    static std::shared_ptr<Texture> createFlatNormal(int resolution = 4);

    void bind(unsigned int unit) const;
    static void unbind(unsigned int unit);

    [[nodiscard]] unsigned int id() const;
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;

private:
    void release();

    unsigned int m_id = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace Engine
