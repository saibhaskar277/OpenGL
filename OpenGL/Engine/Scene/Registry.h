#pragma once

#include "Engine/Scene/Components.h"
#include "Engine/Scene/Entity.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Engine {

class Registry final {
public:
    Entity createEntity();
    void destroyEntity(Entity entity);
    void clear();

    [[nodiscard]] bool alive(Entity entity) const;
    [[nodiscard]] const std::vector<Entity>& entities() const;

    template <typename T>
    T& add(Entity entity, const T& component = T{}) {
        auto& map = store<T>();
        auto [it, inserted] = map.emplace(entity, component);
        if (!inserted) {
            it->second = component;
        }
        return it->second;
    }

    template <typename T>
    [[nodiscard]] bool has(Entity entity) const {
        const auto& map = store<T>();
        return map.find(entity) != map.end();
    }

    template <typename T>
    T& get(Entity entity) {
        return store<T>().at(entity);
    }

    template <typename T>
    [[nodiscard]] const T& get(Entity entity) const {
        return store<T>().at(entity);
    }

    template <typename T>
    void remove(Entity entity) {
        store<T>().erase(entity);
    }

    template <typename T>
    std::unordered_map<Entity, T>& storage() {
        return store<T>();
    }

    template <typename T>
    const std::unordered_map<Entity, T>& storage() const {
        return store<T>();
    }

private:
    template <typename T>
    std::unordered_map<Entity, T>& store();

    template <typename T>
    const std::unordered_map<Entity, T>& store() const {
        return const_cast<Registry*>(this)->store<T>();
    }

    Entity m_nextEntity = 1;
    std::vector<Entity> m_entities;
    std::unordered_set<Entity> m_alive;

    std::unordered_map<Entity, TagComponent> m_tags;
    std::unordered_map<Entity, TransformComponent> m_transforms;
    std::unordered_map<Entity, MeshRendererComponent> m_meshRenderers;
    std::unordered_map<Entity, MaterialComponent> m_materials;
    std::unordered_map<Entity, DirectionalLightComponent> m_directionalLights;
    std::unordered_map<Entity, PointLightComponent> m_pointLights;
    std::unordered_map<Entity, SpotLightComponent> m_spotLights;
    std::unordered_map<Entity, HemisphereLightComponent> m_hemisphereLights;
    std::unordered_map<Entity, RigidBodyComponent> m_rigidBodies;
    std::unordered_map<Entity, ColliderComponent> m_colliders;
};

template <>
inline std::unordered_map<Entity, TagComponent>& Registry::store<TagComponent>() {
    return m_tags;
}
template <>
inline std::unordered_map<Entity, TransformComponent>& Registry::store<TransformComponent>() {
    return m_transforms;
}
template <>
inline std::unordered_map<Entity, MeshRendererComponent>& Registry::store<MeshRendererComponent>() {
    return m_meshRenderers;
}
template <>
inline std::unordered_map<Entity, MaterialComponent>& Registry::store<MaterialComponent>() {
    return m_materials;
}
template <>
inline std::unordered_map<Entity, DirectionalLightComponent>& Registry::store<DirectionalLightComponent>() {
    return m_directionalLights;
}
template <>
inline std::unordered_map<Entity, PointLightComponent>& Registry::store<PointLightComponent>() {
    return m_pointLights;
}
template <>
inline std::unordered_map<Entity, SpotLightComponent>& Registry::store<SpotLightComponent>() {
    return m_spotLights;
}
template <>
inline std::unordered_map<Entity, HemisphereLightComponent>& Registry::store<HemisphereLightComponent>() {
    return m_hemisphereLights;
}
template <>
inline std::unordered_map<Entity, RigidBodyComponent>& Registry::store<RigidBodyComponent>() {
    return m_rigidBodies;
}
template <>
inline std::unordered_map<Entity, ColliderComponent>& Registry::store<ColliderComponent>() {
    return m_colliders;
}

} // namespace Engine
