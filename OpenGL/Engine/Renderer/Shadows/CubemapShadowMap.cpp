#include "Engine/Renderer/Shadows/CubemapShadowMap.h"

#include <glad/glad.h>

#include <stdexcept>
#include <utility>

namespace Engine {

CubemapShadowMap::CubemapShadowMap(int resolution)
    : m_resolution(resolution) {
    if (resolution <= 0) {
        throw std::invalid_argument("Cubemap shadow resolution must be positive");
    }

    glGenFramebuffers(1, &m_framebuffer);
    glGenTextures(1, &m_depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_depthCubemap);

    for (unsigned int face = 0; face < 6; ++face) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            0,
            GL_DEPTH_COMPONENT24,
            resolution,
            resolution,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubemapShadowMap::~CubemapShadowMap() {
    release();
}

CubemapShadowMap::CubemapShadowMap(CubemapShadowMap&& other) noexcept
    : m_framebuffer(std::exchange(other.m_framebuffer, 0)),
      m_depthCubemap(std::exchange(other.m_depthCubemap, 0)),
      m_resolution(std::exchange(other.m_resolution, 0)) {
}

CubemapShadowMap& CubemapShadowMap::operator=(CubemapShadowMap&& other) noexcept {
    if (this != &other) {
        release();
        m_framebuffer = std::exchange(other.m_framebuffer, 0);
        m_depthCubemap = std::exchange(other.m_depthCubemap, 0);
        m_resolution = std::exchange(other.m_resolution, 0);
    }
    return *this;
}

void CubemapShadowMap::beginFace(int face) const {
    if (face < 0 || face > 5) {
        throw std::out_of_range("Cubemap face must be in [0, 5]");
    }

    glViewport(0, 0, m_resolution, m_resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<unsigned int>(face),
        m_depthCubemap,
        0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Point-light shadow framebuffer is incomplete");
    }

    glClear(GL_DEPTH_BUFFER_BIT);
}

void CubemapShadowMap::endWrite(int viewportWidth, int viewportHeight) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, viewportWidth, viewportHeight);
}

void CubemapShadowMap::bindTexture(unsigned int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_depthCubemap);
}

int CubemapShadowMap::resolution() const {
    return m_resolution;
}

void CubemapShadowMap::release() {
    if (m_depthCubemap != 0) {
        glDeleteTextures(1, &m_depthCubemap);
        m_depthCubemap = 0;
    }
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        m_framebuffer = 0;
    }
}

} // namespace Engine
