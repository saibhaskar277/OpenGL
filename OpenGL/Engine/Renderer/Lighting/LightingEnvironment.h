#pragma once

#include "Engine/Renderer/Lighting/DirectionalLight.h"
#include "Engine/Renderer/Lighting/HemisphereLight.h"
#include "Engine/Renderer/Lighting/PointLight.h"
#include "Engine/Renderer/Lighting/SpotLight.h"
#include "Engine/Renderer/RHI/Material.h"

#include <glm/glm.hpp>

#include <cstddef>
#include <vector>

namespace Engine {

class Shader;

class LightingEnvironment final {
public:
    static constexpr std::size_t MaxPointLights = 4;

    [[nodiscard]] DirectionalLight& directionalLight();
    [[nodiscard]] const DirectionalLight& directionalLight() const;
    [[nodiscard]] SpotLight& spotLight();
    [[nodiscard]] const SpotLight& spotLight() const;
    [[nodiscard]] HemisphereLight& hemisphereLight();
    [[nodiscard]] const HemisphereLight& hemisphereLight() const;
    [[nodiscard]] std::vector<PointLight>& pointLights();
    [[nodiscard]] const std::vector<PointLight>& pointLights() const;

    void addPointLight(const PointLight& light);
    void clearPointLights();

    void apply(
        Shader& shader,
        const glm::vec3& viewPosition,
        const Material& material) const;

private:
    DirectionalLight m_directionalLight;
    SpotLight m_spotLight;
    HemisphereLight m_hemisphereLight;
    std::vector<PointLight> m_pointLights;
};

} // namespace Engine
