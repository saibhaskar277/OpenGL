#include "Editor/Viewport/ColliderDebugOverlay.h"

#include "Engine/Physics/PhysicsComponents.h"
#include "Engine/Physics/PhysicsMath.h"
#include "Engine/Scene/Components.h"

#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <vector>

namespace Editor {
namespace {

bool projectWorldToScreen(
    const Engine::PerspectiveCamera& camera,
    const glm::vec3& world,
    float viewportWidth,
    float viewportHeight,
    ImVec2& outScreen) {
    const glm::vec4 clip =
        camera.projectionMatrix() * camera.viewMatrix() * glm::vec4(world, 1.0f);
    if (clip.w <= 0.0001f) {
        return false;
    }

    const glm::vec3 ndc = glm::vec3(clip) / clip.w;
    if (ndc.z < -1.0f || ndc.z > 1.0f) {
        return false;
    }

    outScreen.x = (ndc.x * 0.5f + 0.5f) * viewportWidth;
    outScreen.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * viewportHeight;
    return true;
}

void drawLine(
    ImDrawList* drawList,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight,
    const glm::vec3& a,
    const glm::vec3& b,
    ImU32 color,
    float thickness) {
    ImVec2 sa;
    ImVec2 sb;
    if (!projectWorldToScreen(camera, a, viewportWidth, viewportHeight, sa) ||
        !projectWorldToScreen(camera, b, viewportWidth, viewportHeight, sb)) {
        return;
    }
    drawList->AddLine(sa, sb, color, thickness);
}

glm::mat3 rotationMatrix(const glm::vec3& rotationDegrees) {
    return glm::mat3_cast(Engine::PhysicsMath::rotationDegreesToQuat(rotationDegrees));
}

void drawBoxWire(
    ImDrawList* drawList,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight,
    const glm::vec3& center,
    const glm::vec3& rotationDegrees,
    const glm::vec3& halfExtents,
    const glm::vec3& localOffset,
    ImU32 color,
    float thickness) {
    const glm::mat3 rot = rotationMatrix(rotationDegrees);
    const glm::vec3 origin = center + rot * localOffset;

    glm::vec3 corners[8];
    int i = 0;
    for (int x : {-1, 1}) {
        for (int y : {-1, 1}) {
            for (int z : {-1, 1}) {
                const glm::vec3 local(
                    halfExtents.x * static_cast<float>(x),
                    halfExtents.y * static_cast<float>(y),
                    halfExtents.z * static_cast<float>(z));
                corners[i++] = origin + rot * local;
            }
        }
    }

    // Corner order: (- - -)(- - +)(- + -)(- + +)(+ - -)(+ - +)(+ + -)(+ + +)
    const int edges[12][2] = {
        {0, 1}, {0, 2}, {0, 4},
        {1, 3}, {1, 5},
        {2, 3}, {2, 6},
        {3, 7},
        {4, 5}, {4, 6},
        {5, 7},
        {6, 7}};

    for (const auto& edge : edges) {
        drawLine(
            drawList,
            camera,
            viewportWidth,
            viewportHeight,
            corners[edge[0]],
            corners[edge[1]],
            color,
            thickness);
    }
}

void drawSphereWire(
    ImDrawList* drawList,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight,
    const glm::vec3& center,
    const glm::vec3& rotationDegrees,
    float radius,
    const glm::vec3& localOffset,
    ImU32 color,
    float thickness) {
    const glm::mat3 rot = rotationMatrix(rotationDegrees);
    const glm::vec3 origin = center + rot * localOffset;
    constexpr int segments = 24;

    auto ring = [&](const glm::vec3& axisA, const glm::vec3& axisB) {
        for (int i = 0; i < segments; ++i) {
            const float t0 = (static_cast<float>(i) / segments) * glm::two_pi<float>();
            const float t1 = (static_cast<float>(i + 1) / segments) * glm::two_pi<float>();
            const glm::vec3 p0 =
                origin + rot * (axisA * std::cos(t0) + axisB * std::sin(t0)) * radius;
            const glm::vec3 p1 =
                origin + rot * (axisA * std::cos(t1) + axisB * std::sin(t1)) * radius;
            drawLine(
                drawList,
                camera,
                viewportWidth,
                viewportHeight,
                p0,
                p1,
                color,
                thickness);
        }
    };

    ring(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0));
    ring(glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));
    ring(glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
}

void drawCapsuleWire(
    ImDrawList* drawList,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight,
    const glm::vec3& center,
    const glm::vec3& rotationDegrees,
    float radius,
    float halfHeight,
    const glm::vec3& localOffset,
    ImU32 color,
    float thickness) {
    const glm::mat3 rot = rotationMatrix(rotationDegrees);
    const glm::vec3 origin = center + rot * localOffset;
    const glm::vec3 up = rot * glm::vec3(0, 1, 0);
    const glm::vec3 top = origin + up * halfHeight;
    const glm::vec3 bottom = origin - up * halfHeight;

    drawSphereWire(
        drawList,
        camera,
        viewportWidth,
        viewportHeight,
        top,
        rotationDegrees,
        radius,
        glm::vec3(0.0f),
        color,
        thickness);
    drawSphereWire(
        drawList,
        camera,
        viewportWidth,
        viewportHeight,
        bottom,
        rotationDegrees,
        radius,
        glm::vec3(0.0f),
        color,
        thickness);

    const glm::vec3 right = rot * glm::vec3(1, 0, 0);
    const glm::vec3 forward = rot * glm::vec3(0, 0, 1);
    const glm::vec3 offsets[4] = {
        right * radius,
        -right * radius,
        forward * radius,
        -forward * radius};
    for (const glm::vec3& offset : offsets) {
        drawLine(
            drawList,
            camera,
            viewportWidth,
            viewportHeight,
            top + offset,
            bottom + offset,
            color,
            thickness);
    }
}

} // namespace

void drawColliderDebugGizmos(
    Engine::Scene& scene,
    const Engine::PerspectiveCamera& camera,
    float viewportWidth,
    float viewportHeight) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    Engine::Registry& registry = scene.registry();
    const Engine::Entity selected = scene.selectedEntity();

    for (const auto& [entity, collider] : registry.storage<Engine::ColliderComponent>()) {
        if (!registry.has<Engine::TransformComponent>(entity)) {
            continue;
        }

        const Engine::Transform& transform =
            registry.get<Engine::TransformComponent>(entity).transform;
        const bool isSelected = entity == selected;
        const ImU32 color = isSelected
            ? IM_COL32(80, 220, 120, 255)
            : IM_COL32(80, 180, 255, 180);
        const float thickness = isSelected ? 2.0f : 1.25f;

        switch (collider.shape) {
        case Engine::ColliderShapeType::Sphere:
            drawSphereWire(
                drawList,
                camera,
                viewportWidth,
                viewportHeight,
                transform.position(),
                transform.rotationDegrees(),
                collider.sphereRadius,
                collider.localOffset,
                color,
                thickness);
            break;
        case Engine::ColliderShapeType::Capsule:
            drawCapsuleWire(
                drawList,
                camera,
                viewportWidth,
                viewportHeight,
                transform.position(),
                transform.rotationDegrees(),
                collider.capsuleRadius,
                collider.capsuleHalfHeight,
                collider.localOffset,
                color,
                thickness);
            break;
        case Engine::ColliderShapeType::Box:
        default:
            drawBoxWire(
                drawList,
                camera,
                viewportWidth,
                viewportHeight,
                transform.position(),
                transform.rotationDegrees(),
                collider.boxHalfExtents,
                collider.localOffset,
                color,
                thickness);
            break;
        }
    }
}

} // namespace Editor
