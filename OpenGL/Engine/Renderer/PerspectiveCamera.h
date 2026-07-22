#pragma once

#include <glm/glm.hpp>

namespace Engine {

class PerspectiveCamera final {
public:
    PerspectiveCamera(
        float verticalFieldOfViewDegrees,
        float aspectRatio,
        float nearClip,
        float farClip);

    [[nodiscard]] const glm::vec3& position() const;
    [[nodiscard]] float yawDegrees() const;
    [[nodiscard]] float pitchDegrees() const;
    [[nodiscard]] glm::vec3 forward() const;
    [[nodiscard]] glm::vec3 right() const;
    [[nodiscard]] glm::vec3 up() const;

    void setPosition(const glm::vec3& position);
    void translate(const glm::vec3& offset);
    void setOrientation(float yawDegrees, float pitchDegrees);
    void setAspectRatio(float aspectRatio);
    void setPerspective(
        float verticalFieldOfViewDegrees,
        float nearClip,
        float farClip);

    [[nodiscard]] const glm::mat4& viewMatrix() const;
    [[nodiscard]] const glm::mat4& projectionMatrix() const;

private:
    void updateViewMatrix() const;
    void updateProjectionMatrix() const;

    glm::vec3 m_position{0.0f, 0.0f, 3.0f};
    float m_yawDegrees = -90.0f;
    float m_pitchDegrees = 0.0f;
    float m_fieldOfViewDegrees = 45.0f;
    float m_aspectRatio = 4.0f / 3.0f;
    float m_nearClip = 0.1f;
    float m_farClip = 100.0f;

    mutable glm::mat4 m_viewMatrix{1.0f};
    mutable glm::mat4 m_projectionMatrix{1.0f};
    mutable bool m_viewDirty = true;
    mutable bool m_projectionDirty = true;
};

} // namespace Engine
