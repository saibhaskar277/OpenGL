#include "Engine/Renderer/RHI/Texture.h"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdexcept>
#include <utility>

namespace Engine {
namespace {

GLint wrapToGl(TextureWrap wrap) {
    switch (wrap) {
    case TextureWrap::ClampToEdge:
        return GL_CLAMP_TO_EDGE;
    case TextureWrap::MirroredRepeat:
        return GL_MIRRORED_REPEAT;
    case TextureWrap::Repeat:
    default:
        return GL_REPEAT;
    }
}

} // namespace

Texture::Texture(
    int width,
    int height,
    int channels,
    const unsigned char* pixels,
    TextureColorSpace colorSpace,
    TextureWrap wrap,
    TextureFilter filter)
    : m_width(width),
      m_height(height) {
    if (width <= 0 || height <= 0 || pixels == nullptr) {
        throw std::invalid_argument("Invalid texture dimensions or pixel data");
    }
    if (channels < 1 || channels > 4) {
        throw std::invalid_argument("Texture channels must be between 1 and 4");
    }

    GLenum format = GL_RGBA;
    GLenum internalFormat = GL_RGBA8;
    switch (channels) {
    case 1:
        format = GL_RED;
        internalFormat = GL_R8;
        break;
    case 2:
        format = GL_RG;
        internalFormat = GL_RG8;
        break;
    case 3:
        format = GL_RGB;
        internalFormat =
            colorSpace == TextureColorSpace::Srgb ? GL_SRGB8 : GL_RGB8;
        break;
    case 4:
        format = GL_RGBA;
        internalFormat =
            colorSpace == TextureColorSpace::Srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
        break;
    default:
        break;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalFormat,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        pixels);

    const GLint wrapMode = wrapToGl(wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    switch (filter) {
    case TextureFilter::Nearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case TextureFilter::Linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case TextureFilter::LinearMipmap:
    default:
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    release();
}

Texture::Texture(Texture&& other) noexcept
    : m_id(std::exchange(other.m_id, 0)),
      m_width(std::exchange(other.m_width, 0)),
      m_height(std::exchange(other.m_height, 0)) {
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        release();
        m_id = std::exchange(other.m_id, 0);
        m_width = std::exchange(other.m_width, 0);
        m_height = std::exchange(other.m_height, 0);
    }
    return *this;
}

Texture Texture::fromFile(
    const std::filesystem::path& path,
    TextureColorSpace colorSpace,
    TextureWrap wrap,
    TextureFilter filter) {
    stbi_set_flip_vertically_on_load(1);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(
        path.string().c_str(),
        &width,
        &height,
        &channels,
        0);
    if (pixels == nullptr) {
        throw std::runtime_error(
            "Failed to load texture: " + path.string() +
            " (" + std::string(stbi_failure_reason()) + ")");
    }

    try {
        Texture texture(width, height, channels, pixels, colorSpace, wrap, filter);
        stbi_image_free(pixels);
        return texture;
    } catch (...) {
        stbi_image_free(pixels);
        throw;
    }
}

std::shared_ptr<Texture> Texture::createSolid(
    unsigned char red,
    unsigned char green,
    unsigned char blue,
    unsigned char alpha,
    TextureColorSpace colorSpace) {
    const unsigned char pixels[4] = {red, green, blue, alpha};
    return std::make_shared<Texture>(
        1,
        1,
        4,
        pixels,
        colorSpace,
        TextureWrap::Repeat,
        TextureFilter::Nearest);
}

std::shared_ptr<Texture> Texture::createCheckerboard(
    int resolution,
    int checkerSize,
    unsigned char colorA,
    unsigned char colorB) {
    if (resolution <= 0 || checkerSize <= 0) {
        throw std::invalid_argument("Invalid checkerboard parameters");
    }

    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(resolution) * static_cast<std::size_t>(resolution) * 4u);
    for (int y = 0; y < resolution; ++y) {
        for (int x = 0; x < resolution; ++x) {
            const bool dark =
                ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            const unsigned char value = dark ? colorA : colorB;
            const std::size_t index =
                (static_cast<std::size_t>(y) * static_cast<std::size_t>(resolution) +
                 static_cast<std::size_t>(x)) *
                4u;
            pixels[index + 0] = value;
            pixels[index + 1] = value;
            pixels[index + 2] = value;
            pixels[index + 3] = 255;
        }
    }

    return std::make_shared<Texture>(
        resolution,
        resolution,
        4,
        pixels.data(),
        TextureColorSpace::Srgb,
        TextureWrap::Repeat,
        TextureFilter::LinearMipmap);
}

std::shared_ptr<Texture> Texture::createFlatNormal(int resolution) {
    if (resolution <= 0) {
        throw std::invalid_argument("Invalid flat-normal resolution");
    }

    std::vector<unsigned char> pixels(
        static_cast<std::size_t>(resolution) * static_cast<std::size_t>(resolution) * 3u,
        0);
    for (std::size_t i = 0; i < pixels.size(); i += 3) {
        pixels[i + 0] = 128; // +X in tangent space
        pixels[i + 1] = 128; // +Y
        pixels[i + 2] = 255; // +Z
    }

    return std::make_shared<Texture>(
        resolution,
        resolution,
        3,
        pixels.data(),
        TextureColorSpace::Linear,
        TextureWrap::Repeat,
        TextureFilter::Linear);
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind(unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int Texture::id() const {
    return m_id;
}

int Texture::width() const {
    return m_width;
}

int Texture::height() const {
    return m_height;
}

void Texture::release() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
    m_width = 0;
    m_height = 0;
}

} // namespace Engine
