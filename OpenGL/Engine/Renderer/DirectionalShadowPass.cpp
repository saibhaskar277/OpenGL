#include "Engine/Renderer/DirectionalShadowPass.h"

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Model.h"
#include "Engine/Renderer/Renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>

namespace Engine {

DirectionalShadowPass::DirectionalShadowPass(
    int resolution,
    const std::filesystem::path& depthVertexShader,
    const std::filesystem::path& depthFragmentShader,
    float projectionHalfSize,
    float lightDistance,
    float nearClip,
    float farClip)
    : m_shadowMap(resolution, resolution),
      m_depthShader(depthVertexShader, depthFragmentShader),
      m_projectionHalfSize(projectionHalfSize),
      m_lightDistance(lightDistance),
      m_nearClip(nearClip),
      m_farClip(farClip) {
    if (projectionHalfSize <= 0.0f ||
        lightDistance <= 0.0f ||
        nearClip <= 0.0f ||
        farClip <= nearClip) {
        throw std::invalid_argument("Invalid directional shadow-pass parameters");
    }
}

void DirectionalShadowPass::begin(
    const glm::vec3& lightDirection,
    const glm::vec3& sceneCenter) {
    if (m_active) {
        throw std::logic_error("Directional shadow pass is already active");
    }
    if (glm::dot(lightDirection, lightDirection) <= 0.000001f) {
        throw std::invalid_argument("Shadow-casting light direction cannot be zero");
    }

    const glm::vec3 direction = glm::normalize(lightDirection);
    const glm::vec3 lightPosition = sceneCenter - direction * m_lightDistance;
    const glm::vec3 worldUp =
        std::abs(glm::dot(direction, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
        ? glm::vec3(0.0f, 0.0f, 1.0f)
        : glm::vec3(0.0f, 1.0f, 0.0f);

    const glm::mat4 lightView = glm::lookAt(lightPosition, sceneCenter, worldUp);
    const glm::mat4 lightProjection = glm::ortho(
        -m_projectionHalfSize,
        m_projectionHalfSize,
        -m_projectionHalfSize,
        m_projectionHalfSize,
        m_nearClip,
        m_farClip);
    m_lightSpaceMatrix = lightProjection * lightView;

    m_shadowMap.beginWrite();
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);
    m_depthShader.bind();
    m_depthShader.setMat4("uLightSpaceMatrix", m_lightSpaceMatrix);
    m_active = true;
}

void DirectionalShadowPass::draw(
    const Mesh& mesh,
    const glm::mat4& modelMatrix) {
    if (!m_active) {
        throw std::logic_error("Directional shadow pass has not begun");
    }

    m_depthShader.setMat4("uModel", modelMatrix);
    Renderer::draw(mesh, m_depthShader);
}

void DirectionalShadowPass::draw(
    const Model& model,
    const glm::mat4& modelMatrix) {
    for (const ModelPart& part : model.parts()) {
        draw(part.mesh, modelMatrix);
    }
}

void DirectionalShadowPass::end(int viewportWidth, int viewportHeight) {
    if (!m_active) {
        throw std::logic_error("Directional shadow pass has not begun");
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    ShadowMap::endWrite(viewportWidth, viewportHeight);
    m_active = false;
}

void DirectionalShadowPass::bindForLighting(
    Shader& lightingShader,
    unsigned int textureUnit) const {
    m_shadowMap.bindTexture(textureUnit);
    lightingShader.bind();
    lightingShader.setInt(
        "uDirectionalShadowMap",
        static_cast<int>(textureUnit));
    lightingShader.setMat4("uLightSpaceMatrix", m_lightSpaceMatrix);
}

const glm::mat4& DirectionalShadowPass::lightSpaceMatrix() const {
    return m_lightSpaceMatrix;
}

} // namespace Engine
