#pragma once

#include "Engine/Assets/AssetManager.h"
#include "Engine/Renderer/Shadows/LightShadowSystem.h"
#include "Engine/Renderer/Lighting/LightingEnvironment.h"
#include "Engine/Renderer/RHI/Material.h"
#include "Engine/Renderer/Mesh/Mesh.h"
#include "Engine/Renderer/Mesh/Model.h"
#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Renderer/RHI/Shader.h"
#include "Engine/Scene/Components.h"
#include "Engine/Scene/Scene.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace Engine {

class SceneRenderer final {
public:
    SceneRenderer();

    void render(
        Scene& scene,
        AssetManager& assets,
        LightShadowSystem& shadows,
        Shader& litShader,
        PerspectiveCamera& camera,
        int viewportWidth,
        int viewportHeight);

private:
    Material resolveMaterial(const MaterialComponent& component, AssetManager& assets);
    Mesh& cubeMesh();
    Model* modelForPath(AssetManager& assets, const std::string& relativePath);

    std::unique_ptr<Mesh> m_cubeMesh;
    std::unordered_map<std::string, std::unique_ptr<Model>> m_modelCache;
    LightingEnvironment m_lighting;
};

} // namespace Engine
