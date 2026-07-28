#include "Engine/Scene/Registry.h"

#include <algorithm>

namespace Engine {

Entity Registry::createEntity() {
    const Entity entity = m_nextEntity++;
    m_entities.push_back(entity);
    m_alive.insert(entity);
    return entity;
}

void Registry::destroyEntity(Entity entity) {
    if (!alive(entity)) {
        return;
    }

    m_tags.erase(entity);
    m_transforms.erase(entity);
    m_meshRenderers.erase(entity);
    m_materials.erase(entity);
    m_directionalLights.erase(entity);
    m_pointLights.erase(entity);
    m_spotLights.erase(entity);
    m_hemisphereLights.erase(entity);

    m_alive.erase(entity);
    m_entities.erase(
        std::remove(m_entities.begin(), m_entities.end(), entity),
        m_entities.end());
}

void Registry::clear() {
    m_tags.clear();
    m_transforms.clear();
    m_meshRenderers.clear();
    m_materials.clear();
    m_directionalLights.clear();
    m_pointLights.clear();
    m_spotLights.clear();
    m_hemisphereLights.clear();
    m_entities.clear();
    m_alive.clear();
    m_nextEntity = 1;
}

bool Registry::alive(Entity entity) const {
    return m_alive.find(entity) != m_alive.end();
}

const std::vector<Entity>& Registry::entities() const {
    return m_entities;
}

} // namespace Engine
