#pragma once

#include "Engine/Renderer/RHI/Shader.h"
#include "Engine/Renderer/Shadows/ShadowMap.h"

#include <glm/glm.hpp>

#include <filesystem>

namespace Engine {

class Mesh;
class Model;

class DirectionalShadowPass final {
public:
    DirectionalShadowPass(
        int resolution,
        const std::filesystem::path& depthVertexShader,
        const std::filesystem::path& depthFragmentShader,
        float projectionHalfSize = 8.0f,
        float lightDistance = 12.0f,
        float nearClip = 0.1f,
        float farClip = 30.0f);

    void begin(const glm::vec3& lightDirection, const glm::vec3& sceneCenter);
    void draw(const Mesh& mesh, const glm::mat4& modelMatrix);
    void draw(const Model& model, const glm::mat4& modelMatrix);
    void end(int viewportWidth, int viewportHeight);
    void bindForLighting(Shader& lightingShader, unsigned int textureUnit) const;

    [[nodiscard]] const glm::mat4& lightSpaceMatrix() const;

private:
    ShadowMap m_shadowMap;
    Shader m_depthShader;
    float m_projectionHalfSize;
    float m_lightDistance;
    float m_nearClip;
    float m_farClip;
    glm::mat4 m_lightSpaceMatrix{1.0f};
    bool m_active = false;
};

} // namespace Engine
