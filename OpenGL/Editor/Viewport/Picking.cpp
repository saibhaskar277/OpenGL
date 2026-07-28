#include "Editor/Viewport/Picking.h"

#include "Engine/Scene/Components.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace Editor {
namespace {

bool intersectAabb(
    const Ray& ray,
    const glm::vec3& minBound,
    const glm::vec3& maxBound,
    float& distance) {
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    for (int axis = 0; axis < 3; ++axis) {
        const float origin = ray.origin[axis];
        const float direction = ray.direction[axis];
        float minB = minBound[axis];
        float maxB = maxBound[axis];

        if (std::abs(direction) < 0.000001f) {
            if (origin < minB || origin > maxB) {
                return false;
            }
            continue;
        }

        float t1 = (minB - origin) / direction;
        float t2 = (maxB - origin) / direction;
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) {
            return false;
        }
    }

    distance = tMin;
    return tMin >= 0.0f;
}

bool intersectSphere(
    const Ray& ray,
    const glm::vec3& center,
    float radius,
    float& distance) {
    const glm::vec3 oc = ray.origin - center;
    const float a = glm::dot(ray.direction, ray.direction);
    const float b = 2.0f * glm::dot(oc, ray.direction);
    const float c = glm::dot(oc, oc) - radius * radius;
    const float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) {
        return false;
    }

    const float sqrtD = std::sqrt(discriminant);
    const float t0 = (-b - sqrtD) / (2.0f * a);
    const float t1 = (-b + sqrtD) / (2.0f * a);
    const float t = t0 >= 0.0f ? t0 : t1;
    if (t < 0.0f) {
        return false;
    }
    distance = t;
    return true;
}

glm::vec3 lightWorldPosition(const Engine::Registry& registry, Engine::Entity entity) {
    if (registry.has<Engine::PointLightComponent>(entity)) {
        return registry.get<Engine::PointLightComponent>(entity).light.position;
    }
    if (registry.has<Engine::SpotLightComponent>(entity)) {
        return registry.get<Engine::SpotLightComponent>(entity).light.position;
    }
    if (registry.has<Engine::TransformComponent>(entity)) {
        return registry.get<Engine::TransformComponent>(entity).transform.position();
    }
    return glm::vec3(0.0f);
}

} // namespace

Ray screenPointToRay(
    const Engine::PerspectiveCamera& camera,
    float mouseX,
    float mouseY,
    float viewportWidth,
    float viewportHeight) {
    const float x = (2.0f * mouseX) / viewportWidth - 1.0f;
    const float y = 1.0f - (2.0f * mouseY) / viewportHeight;

    const glm::mat4 inverseProjection = glm::inverse(camera.projectionMatrix());
    const glm::mat4 inverseView = glm::inverse(camera.viewMatrix());

    glm::vec4 eye = inverseProjection * glm::vec4(x, y, -1.0f, 1.0f);
    eye = glm::vec4(eye.x, eye.y, -1.0f, 0.0f);
    const glm::vec3 worldDirection = glm::normalize(glm::vec3(inverseView * eye));

    Ray ray;
    ray.origin = camera.position();
    ray.direction = worldDirection;
    return ray;
}

Engine::Entity pickEntity(
    Engine::Scene& scene,
    const Engine::PerspectiveCamera& camera,
    float mouseX,
    float mouseY,
    float viewportWidth,
    float viewportHeight) {
    const Ray ray = screenPointToRay(camera, mouseX, mouseY, viewportWidth, viewportHeight);
    Engine::Registry& registry = scene.registry();

    Engine::Entity closestEntity = Engine::NullEntity;
    float closestDistance = std::numeric_limits<float>::max();

    for (const auto& [entity, mesh] : registry.storage<Engine::MeshRendererComponent>()) {
        if (!mesh.visible || !registry.has<Engine::TransformComponent>(entity)) {
            continue;
        }

        const Engine::Transform& transform =
            registry.get<Engine::TransformComponent>(entity).transform;
        const glm::vec3 center = transform.position();
        const glm::vec3 halfExtents = transform.scale() * 0.5f;
        float distance = 0.0f;
        if (intersectAabb(ray, center - halfExtents, center + halfExtents, distance) &&
            distance < closestDistance) {
            closestDistance = distance;
            closestEntity = entity;
        }
    }

    constexpr float lightPickRadius = 0.35f;
    for (Engine::Entity entity : registry.entities()) {
        const bool isLight =
            registry.has<Engine::PointLightComponent>(entity) ||
            registry.has<Engine::SpotLightComponent>(entity) ||
            registry.has<Engine::DirectionalLightComponent>(entity);
        if (!isLight) {
            continue;
        }

        float distance = 0.0f;
        if (intersectSphere(ray, lightWorldPosition(registry, entity), lightPickRadius, distance) &&
            distance < closestDistance) {
            closestDistance = distance;
            closestEntity = entity;
        }
    }

    return closestEntity;
}

} // namespace Editor
