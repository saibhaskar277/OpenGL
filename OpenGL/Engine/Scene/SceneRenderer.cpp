#include "Engine/Scene/SceneRenderer.h"

#include "Engine/Renderer/Mesh/PrimitiveFactory.h"
#include "Engine/Renderer/RHI/Renderer.h"
#include "Engine/Scene/Components.h"

#include <filesystem>
#include <stdexcept>

namespace Engine {

SceneRenderer::SceneRenderer() = default;

Mesh& SceneRenderer::cubeMesh() {
    if (!m_cubeMesh) {
        m_cubeMesh = std::make_unique<Mesh>(PrimitiveFactory::createCube());
    }
    return *m_cubeMesh;
}

Model* SceneRenderer::modelForPath(AssetManager& assets, const std::string& relativePath) {
    if (relativePath.empty()) {
        return nullptr;
    }
    const auto found = m_modelCache.find(relativePath);
    if (found != m_modelCache.end()) {
        return found->second.get();
    }
    try {
        const std::filesystem::path fullPath = assets.root() / relativePath;
        auto model = std::make_unique<Model>(Model::loadObj(fullPath));
        Model* raw = model.get();
        m_modelCache.emplace(relativePath, std::move(model));
        return raw;
    } catch (const std::exception&) {
        return nullptr;
    }
}

Material SceneRenderer::resolveMaterial(
    const MaterialComponent& component,
    AssetManager& assets) {
    Material material;
    material.albedo = component.albedo;
    material.specularColor = component.specularColor;
    material.shininess = component.shininess;
    material.emissive = component.emissive;
    material.ambientOcclusion = component.ambientOcclusion;
    material.albedoMap = assets.getTexture(component.albedoMapPath, TextureColorSpace::Srgb);
    material.normalMap = assets.getTexture(component.normalMapPath, TextureColorSpace::Linear);
    material.specularMap = assets.getTexture(component.specularMapPath, TextureColorSpace::Linear);
    material.emissiveMap = assets.getTexture(component.emissiveMapPath, TextureColorSpace::Srgb);
    material.aoMap = assets.getTexture(component.aoMapPath, TextureColorSpace::Linear);
    return material;
}

void SceneRenderer::render(
    Scene& scene,
    AssetManager& assets,
    LightShadowSystem& shadows,
    Shader& litShader,
    PerspectiveCamera& camera,
    int viewportWidth,
    int viewportHeight) {
    Registry& registry = scene.registry();

    // Apply spot lights that follow the camera before building lighting.
    for (auto& [entity, spot] : registry.storage<SpotLightComponent>()) {
        (void)entity;
        if (spot.followCamera) {
            spot.light.position = camera.position();
            spot.light.direction = camera.forward();
        }
    }

    scene.buildLightingEnvironment(m_lighting);

    shadows.render(
        m_lighting,
        glm::vec3(0.0f, -0.25f, 0.0f),
        viewportWidth,
        viewportHeight,
        [&](LightShadowSystem::CasterDrawer& draw) {
            for (const auto& [entity, meshRenderer] : registry.storage<MeshRendererComponent>()) {
                if (!meshRenderer.castShadows || !meshRenderer.visible) {
                    continue;
                }
                if (!registry.has<TransformComponent>(entity)) {
                    continue;
                }
                const glm::mat4 modelMatrix =
                    registry.get<TransformComponent>(entity).transform.matrix();

                if (meshRenderer.meshType == MeshType::Model) {
                    if (Model* model = modelForPath(assets, meshRenderer.modelPath)) {
                        draw(*model, modelMatrix);
                    }
                } else {
                    draw(cubeMesh(), modelMatrix);
                }
            }
        });

    Renderer::clear();
    litShader.bind();
    litShader.setMat4("uView", camera.viewMatrix());
    litShader.setMat4("uProjection", camera.projectionMatrix());
    shadows.bindForLighting(litShader);

    for (const auto& [entity, meshRenderer] : registry.storage<MeshRendererComponent>()) {
        if (!meshRenderer.visible) {
            continue;
        }
        if (!registry.has<TransformComponent>(entity)) {
            continue;
        }

        Material material;
        if (registry.has<MaterialComponent>(entity)) {
            material = resolveMaterial(registry.get<MaterialComponent>(entity), assets);
        }

        const glm::mat4 modelMatrix =
            registry.get<TransformComponent>(entity).transform.matrix();
        litShader.setMat4("uModel", modelMatrix);
        m_lighting.apply(litShader, camera.position(), material);
        material.bindMaps(litShader, 1);

        if (meshRenderer.meshType == MeshType::Model) {
            if (Model* model = modelForPath(assets, meshRenderer.modelPath)) {
                for (const ModelPart& part : model->parts()) {
                    // Prefer entity material maps; fall back to model part material colors.
                    Material drawMaterial = material;
                    if (!drawMaterial.albedoMap && part.material.albedoMap) {
                        drawMaterial = part.material;
                    }
                    m_lighting.apply(litShader, camera.position(), drawMaterial);
                    drawMaterial.bindMaps(litShader, 1);
                    Renderer::draw(part.mesh, litShader);
                }
            }
        } else {
            Renderer::draw(cubeMesh(), litShader);
        }
    }
}

} // namespace Engine
