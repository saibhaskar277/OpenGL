#include "Engine/Renderer/Material.h"

#include "Engine/Renderer/Shader.h"

namespace Engine {

void Material::bindMaps(Shader& shader, unsigned int firstUnit) const {
    shader.bind();

    const unsigned int albedoUnit = firstUnit;
    const unsigned int normalUnit = firstUnit + 1;
    const unsigned int specularUnit = firstUnit + 2;
    const unsigned int emissiveUnit = firstUnit + 3;
    const unsigned int aoUnit = firstUnit + 4;

    shader.setInt("uAlbedoMap", static_cast<int>(albedoUnit));
    shader.setInt("uNormalMap", static_cast<int>(normalUnit));
    shader.setInt("uSpecularMap", static_cast<int>(specularUnit));
    shader.setInt("uEmissiveMap", static_cast<int>(emissiveUnit));
    shader.setInt("uAoMap", static_cast<int>(aoUnit));

    shader.setInt("uUseAlbedoMap", albedoMap ? 1 : 0);
    shader.setInt("uUseNormalMap", normalMap ? 1 : 0);
    shader.setInt("uUseSpecularMap", specularMap ? 1 : 0);
    shader.setInt("uUseEmissiveMap", emissiveMap ? 1 : 0);
    shader.setInt("uUseAoMap", aoMap ? 1 : 0);

    if (albedoMap) {
        albedoMap->bind(albedoUnit);
    }
    if (normalMap) {
        normalMap->bind(normalUnit);
    }
    if (specularMap) {
        specularMap->bind(specularUnit);
    }
    if (emissiveMap) {
        emissiveMap->bind(emissiveUnit);
    }
    if (aoMap) {
        aoMap->bind(aoUnit);
    }
}

} // namespace Engine
