#include "Engine/Renderer/Lighting/LightingEnvironment.h"

#include "Engine/Renderer/Shader.h"

#include <glm/trigonometric.hpp>

#include <stdexcept>
#include <string>

namespace Engine {

DirectionalLight& LightingEnvironment::directionalLight() {
    return m_directionalLight;
}

SpotLight& LightingEnvironment::spotLight() {
    return m_spotLight;
}

HemisphereLight& LightingEnvironment::hemisphereLight() {
    return m_hemisphereLight;
}

std::vector<PointLight>& LightingEnvironment::pointLights() {
    return m_pointLights;
}

void LightingEnvironment::addPointLight(const PointLight& light) {
    if (m_pointLights.size() >= MaxPointLights) {
        throw std::length_error("Maximum point-light count exceeded");
    }
    m_pointLights.push_back(light);
}

void LightingEnvironment::clearPointLights() {
    m_pointLights.clear();
}

void LightingEnvironment::apply(
    Shader& shader,
    const glm::vec3& viewPosition,
    const Material& material) const {
    shader.bind();
    shader.setVec3("uViewPosition", viewPosition);

    shader.setVec3("uMaterial.albedo", material.albedo);
    shader.setVec3("uMaterial.specularColor", material.specularColor);
    shader.setFloat("uMaterial.shininess", material.shininess);

    shader.setInt("uDirectional.enabled", m_directionalLight.enabled ? 1 : 0);
    shader.setVec3("uDirectional.direction", m_directionalLight.direction);
    shader.setVec3("uDirectional.color", m_directionalLight.color);
    shader.setFloat("uDirectional.intensity", m_directionalLight.intensity);

    shader.setInt("uPointLightCount", static_cast<int>(m_pointLights.size()));
    for (std::size_t index = 0; index < m_pointLights.size(); ++index) {
        const PointLight& light = m_pointLights[index];
        const std::string prefix = "uPointLights[" + std::to_string(index) + "].";
        shader.setVec3(prefix + "position", light.position);
        shader.setVec3(prefix + "color", light.color);
        shader.setFloat(prefix + "intensity", light.intensity);
        shader.setFloat(prefix + "constantAttenuation", light.constantAttenuation);
        shader.setFloat(prefix + "linearAttenuation", light.linearAttenuation);
        shader.setFloat(prefix + "quadraticAttenuation", light.quadraticAttenuation);
    }

    shader.setInt("uSpot.enabled", m_spotLight.enabled ? 1 : 0);
    shader.setVec3("uSpot.position", m_spotLight.position);
    shader.setVec3("uSpot.direction", m_spotLight.direction);
    shader.setVec3("uSpot.color", m_spotLight.color);
    shader.setFloat("uSpot.intensity", m_spotLight.intensity);
    shader.setFloat(
        "uSpot.innerCutoff",
        glm::cos(glm::radians(m_spotLight.innerCutoffDegrees)));
    shader.setFloat(
        "uSpot.outerCutoff",
        glm::cos(glm::radians(m_spotLight.outerCutoffDegrees)));
    shader.setFloat("uSpot.constantAttenuation", m_spotLight.constantAttenuation);
    shader.setFloat("uSpot.linearAttenuation", m_spotLight.linearAttenuation);
    shader.setFloat("uSpot.quadraticAttenuation", m_spotLight.quadraticAttenuation);

    shader.setVec3("uHemisphere.skyColor", m_hemisphereLight.skyColor);
    shader.setVec3("uHemisphere.groundColor", m_hemisphereLight.groundColor);
    shader.setFloat("uHemisphere.intensity", m_hemisphereLight.intensity);
    shader.setVec3("uHemisphere.bounceColor", m_hemisphereLight.bounceColor);
    shader.setFloat("uHemisphere.bounceStrength", m_hemisphereLight.bounceStrength);
    shader.setFloat("uHemisphere.floorHeight", m_hemisphereLight.floorHeight);
    shader.setFloat("uHemisphere.bounceFalloff", m_hemisphereLight.bounceFalloff);
}

} // namespace Engine
