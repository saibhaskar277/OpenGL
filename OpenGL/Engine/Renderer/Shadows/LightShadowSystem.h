#pragma once

#include "Engine/Renderer/Shadows/CubemapShadowMap.h"
#include "Engine/Renderer/Shadows/ShadowMap.h"
#include "Engine/Renderer/RHI/Shader.h"

#include <glm/glm.hpp>

#include <array>
#include <functional>
#include <cstddef>

namespace Engine {

class Mesh;
class Model;
class LightingEnvironment;

// Casts shadows for every enabled directional / spot / point light.
// Hemisphere lighting is ambient-only and does not cast shadows.
class LightShadowSystem final {
public:
    class CasterDrawer final {
    public:
        explicit CasterDrawer(Shader& depthShader);

        void draw(const Mesh& mesh, const glm::mat4& modelMatrix);
        void draw(const Model& model, const glm::mat4& modelMatrix);

        void operator()(const Mesh& mesh, const glm::mat4& modelMatrix) {
            draw(mesh, modelMatrix);
        }
        void operator()(const Model& model, const glm::mat4& modelMatrix) {
            draw(model, modelMatrix);
        }

    private:
        Shader& m_depthShader;
    };

    using CasterSubmit = std::function<void(CasterDrawer&)>;

    explicit LightShadowSystem(
        int directionalResolution = 2048,
        int spotResolution = 1024,
        int pointResolution = 512,
        float directionalHalfSize = 6.0f,
        float pointFarPlane = 25.0f,
        float spotFarPlane = 25.0f);

    // Submit each shadow-casting object once; they are drawn into every
    // active light shadow map automatically.
    void render(
        const LightingEnvironment& lighting,
        const glm::vec3& directionalFocus,
        int viewportWidth,
        int viewportHeight,
        const CasterSubmit& submitCasters);

    // Texture units: 0 = directional, 6 = spot, 7..10 = point cubemaps.
    // Units 1..5 stay free for material maps.
    void bindForLighting(Shader& lightingShader) const;

    [[nodiscard]] float pointFarPlane() const;

private:
    static std::array<glm::mat4, 6> pointFaceMatrices(
        const glm::vec3& lightPosition,
        float nearPlane,
        float farPlane);

    ShadowMap m_directionalShadowMap;
    ShadowMap m_spotShadowMap;
    Shader m_depth2DShader;
    Shader m_pointDepthShader;
    std::array<CubemapShadowMap, 4> m_pointShadowMaps;

    float m_directionalHalfSize;
    float m_directionalLightDistance = 12.0f;
    float m_directionalNear = 0.1f;
    float m_directionalFar = 30.0f;
    float m_pointFarPlane;
    float m_spotFarPlane;
    float m_spotNearPlane = 0.1f;
    float m_pointNearPlane = 0.1f;

    glm::mat4 m_directionalLightSpaceMatrix{1.0f};
    glm::mat4 m_spotLightSpaceMatrix{1.0f};
    bool m_directionalActive = false;
    bool m_spotActive = false;
    std::size_t m_pointLightCount = 0;
};

} // namespace Engine
