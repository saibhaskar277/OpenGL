#pragma once

#include "Engine/Renderer/Lighting/LightingEnvironment.h"
#include "Engine/Scene/Entity.h"
#include "Engine/Scene/Registry.h"

#include <string>

namespace Engine {

class Scene final {
public:
    Registry& registry();
    [[nodiscard]] const Registry& registry() const;

    Entity createEntity(const std::string& name = "Entity");
    void destroyEntity(Entity entity);
    void clear();

    Entity createCube(const std::string& name = "Cube");
    Entity createDirectionalLight(const std::string& name = "Directional Light");
    Entity createPointLight(const std::string& name = "Point Light");
    Entity createSpotLight(const std::string& name = "Spot Light");
    Entity createHemisphereLight(const std::string& name = "Hemisphere Light");
    void createDefaultScene();

    // Rebuilds lighting from light components. Spot follow-camera is applied by caller.
    void buildLightingEnvironment(LightingEnvironment& lighting) const;

    [[nodiscard]] Entity selectedEntity() const;
    void setSelectedEntity(Entity entity);

private:
    Registry m_registry;
    Entity m_selectedEntity = NullEntity;
};

} // namespace Engine
