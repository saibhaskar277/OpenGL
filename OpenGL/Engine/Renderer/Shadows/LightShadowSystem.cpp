#include "Engine/Renderer/Shadows/LightShadowSystem.h"

#include "Engine/Renderer/Lighting/LightingEnvironment.h"
#include "Engine/Renderer/Mesh/Mesh.h"
#include "Engine/Renderer/Mesh/Model.h"
#include "Engine/Renderer/RHI/Renderer.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <stdexcept>
#include <string>

namespace Engine {

LightShadowSystem::CasterDrawer::CasterDrawer(Shader& depthShader)
    : m_depthShader(depthShader) {
}

void LightShadowSystem::CasterDrawer::draw(
    const Mesh& mesh,
    const glm::mat4& modelMatrix) {
    m_depthShader.setMat4("uModel", modelMatrix);
    Renderer::draw(mesh, m_depthShader);
}

void LightShadowSystem::CasterDrawer::draw(
    const Model& model,
    const glm::mat4& modelMatrix) {
    for (const ModelPart& part : model.parts()) {
        draw(part.mesh, modelMatrix);
    }
}

LightShadowSystem::LightShadowSystem(
    int directionalResolution,
    int spotResolution,
    int pointResolution,
    float directionalHalfSize,
    float pointFarPlane,
    float spotFarPlane)
    : m_directionalShadowMap(directionalResolution, directionalResolution),
      m_spotShadowMap(spotResolution, spotResolution),
      m_depth2DShader(
          "Assets/Shaders/ShadowDepth.vert",
          "Assets/Shaders/ShadowDepth.frag"),
      m_pointDepthShader(
          "Assets/Shaders/ShadowCubemap.vert",
          "Assets/Shaders/ShadowCubemap.frag"),
      m_pointShadowMaps{
          CubemapShadowMap(pointResolution),
          CubemapShadowMap(pointResolution),
          CubemapShadowMap(pointResolution),
          CubemapShadowMap(pointResolution)},
      m_directionalHalfSize(directionalHalfSize),
      m_pointFarPlane(pointFarPlane),
      m_spotFarPlane(spotFarPlane) {
    if (directionalHalfSize <= 0.0f) {
        throw std::invalid_argument("Directional shadow half-size must be positive");
    }
    if (pointFarPlane <= m_pointNearPlane || spotFarPlane <= m_spotNearPlane) {
        throw std::invalid_argument("Shadow far planes must be greater than near planes");
    }
}

void LightShadowSystem::render(
    const LightingEnvironment& lighting,
    const glm::vec3& directionalFocus,
    int viewportWidth,
    int viewportHeight,
    const CasterSubmit& submitCasters) {
    if (!submitCasters) {
        throw std::invalid_argument("Shadow caster submit callback is required");
    }

    m_directionalActive = false;
    m_spotActive = false;
    m_pointLightCount = lighting.pointLights().size();
    if (m_pointLightCount > m_pointShadowMaps.size()) {
        m_pointLightCount = m_pointShadowMaps.size();
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    // --- Directional ---
    if (lighting.directionalLight().enabled) {
        const glm::vec3 direction = lighting.directionalLight().direction;
        if (glm::dot(direction, direction) <= 0.000001f) {
            throw std::invalid_argument("Directional light direction cannot be zero");
        }

        const glm::vec3 normalized = glm::normalize(direction);
        const glm::vec3 lightPosition =
            directionalFocus - normalized * m_directionalLightDistance;
        const glm::vec3 worldUp =
            std::abs(glm::dot(normalized, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
            ? glm::vec3(0.0f, 0.0f, 1.0f)
            : glm::vec3(0.0f, 1.0f, 0.0f);

        const glm::mat4 lightView = glm::lookAt(lightPosition, directionalFocus, worldUp);
        const glm::mat4 lightProjection = glm::ortho(
            -m_directionalHalfSize,
            m_directionalHalfSize,
            -m_directionalHalfSize,
            m_directionalHalfSize,
            m_directionalNear,
            m_directionalFar);
        m_directionalLightSpaceMatrix = lightProjection * lightView;

        m_directionalShadowMap.beginWrite();
        m_depth2DShader.bind();
        m_depth2DShader.setMat4("uLightSpaceMatrix", m_directionalLightSpaceMatrix);
        {
            CasterDrawer drawer(m_depth2DShader);
            submitCasters(drawer);
        }
        ShadowMap::endWrite(viewportWidth, viewportHeight);
        m_directionalActive = true;
    }

    // --- Spot ---
    if (lighting.spotLight().enabled) {
        const SpotLight& spot = lighting.spotLight();
        const glm::vec3 direction = spot.direction;
        if (glm::dot(direction, direction) <= 0.000001f) {
            throw std::invalid_argument("Spot light direction cannot be zero");
        }

        const float fovDegrees = std::max(spot.outerCutoffDegrees * 2.0f, 25.0f);
        const glm::mat4 lightProjection = glm::perspective(
            glm::radians(fovDegrees),
            1.0f,
            m_spotNearPlane,
            m_spotFarPlane);
        const glm::vec3 normalized = glm::normalize(direction);
        const glm::vec3 worldUp =
            std::abs(glm::dot(normalized, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
            ? glm::vec3(0.0f, 0.0f, 1.0f)
            : glm::vec3(0.0f, 1.0f, 0.0f);
        const glm::mat4 lightView = glm::lookAt(
            spot.position,
            spot.position + normalized,
            worldUp);
        m_spotLightSpaceMatrix = lightProjection * lightView;

        m_spotShadowMap.beginWrite();
        m_depth2DShader.bind();
        m_depth2DShader.setMat4("uLightSpaceMatrix", m_spotLightSpaceMatrix);
        {
            CasterDrawer drawer(m_depth2DShader);
            submitCasters(drawer);
        }
        ShadowMap::endWrite(viewportWidth, viewportHeight);
        m_spotActive = true;
    }

    // --- Point lights (cubemap, 6 faces each) ---
    for (std::size_t lightIndex = 0; lightIndex < m_pointLightCount; ++lightIndex) {
        const PointLight& point = lighting.pointLights()[lightIndex];
        const auto faces = pointFaceMatrices(
            point.position,
            m_pointNearPlane,
            m_pointFarPlane);

        m_pointDepthShader.bind();
        m_pointDepthShader.setVec3("uLightPosition", point.position);
        m_pointDepthShader.setFloat("uFarPlane", m_pointFarPlane);

        for (int face = 0; face < 6; ++face) {
            m_pointShadowMaps[lightIndex].beginFace(face);
            m_pointDepthShader.setMat4("uLightSpaceMatrix", faces[static_cast<std::size_t>(face)]);
            CasterDrawer drawer(m_pointDepthShader);
            submitCasters(drawer);
        }
        CubemapShadowMap::endWrite(viewportWidth, viewportHeight);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void LightShadowSystem::bindForLighting(Shader& lightingShader) const {
    lightingShader.bind();

    constexpr unsigned int directionalUnit = 0;
    constexpr unsigned int spotUnit = 6;
    constexpr unsigned int pointUnitBase = 7;

    lightingShader.setInt("uDirectionalShadowMap", static_cast<int>(directionalUnit));
    lightingShader.setInt("uSpotShadowMap", static_cast<int>(spotUnit));
    lightingShader.setInt("uDirectionalCastsShadow", m_directionalActive ? 1 : 0);
    lightingShader.setInt("uSpotCastsShadow", m_spotActive ? 1 : 0);
    lightingShader.setMat4("uLightSpaceMatrix", m_directionalLightSpaceMatrix);
    lightingShader.setMat4("uSpotLightSpaceMatrix", m_spotLightSpaceMatrix);
    lightingShader.setFloat("uPointShadowFarPlane", m_pointFarPlane);
    lightingShader.setInt(
        "uPointShadowCount",
        static_cast<int>(m_pointLightCount));

    if (m_directionalActive) {
        m_directionalShadowMap.bindTexture(directionalUnit);
    }
    if (m_spotActive) {
        m_spotShadowMap.bindTexture(spotUnit);
    }

    for (std::size_t i = 0; i < m_pointShadowMaps.size(); ++i) {
        const unsigned int unit = pointUnitBase + static_cast<unsigned int>(i);
        lightingShader.setInt(
            "uPointShadowMaps[" + std::to_string(i) + "]",
            static_cast<int>(unit));
        // Always bind a cubemap so the sampler stays valid even if unused.
        m_pointShadowMaps[i].bindTexture(unit);
    }
}

std::array<glm::mat4, 6> LightShadowSystem::pointFaceMatrices(
    const glm::vec3& lightPosition,
    float nearPlane,
    float farPlane) {
    const glm::mat4 projection = glm::perspective(
        glm::radians(90.0f),
        1.0f,
        nearPlane,
        farPlane);

    return {
        projection * glm::lookAt(
            lightPosition,
            lightPosition + glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)),
        projection * glm::lookAt(
            lightPosition,
            lightPosition + glm::vec3(-1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)),
        projection * glm::lookAt(
            lightPosition,
            lightPosition + glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)),
        projection * glm::lookAt(
            lightPosition,
            lightPosition + glm::vec3(0.0f, -1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, -1.0f)),
        projection * glm::lookAt(
            lightPosition,
            lightPosition + glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(0.0f, -1.0f, 0.0f)),
        projection * glm::lookAt(
            lightPosition,
            lightPosition + glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, -1.0f, 0.0f))};
}

float LightShadowSystem::pointFarPlane() const {
    return m_pointFarPlane;
}

} // namespace Engine
