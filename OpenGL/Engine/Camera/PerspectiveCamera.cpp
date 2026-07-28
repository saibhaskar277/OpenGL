#include "Engine/Camera/PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>

namespace Engine {

PerspectiveCamera::PerspectiveCamera(
    float verticalFieldOfViewDegrees,
    float aspectRatio,
    float nearClip,
    float farClip)
    : m_fieldOfViewDegrees(verticalFieldOfViewDegrees),
      m_aspectRatio(aspectRatio),
      m_nearClip(nearClip),
      m_farClip(farClip) {
    if (verticalFieldOfViewDegrees <= 0.0f ||
        verticalFieldOfViewDegrees >= 180.0f ||
        aspectRatio <= 0.0f ||
        nearClip <= 0.0f ||
        farClip <= nearClip) {
        throw std::invalid_argument("Invalid perspective camera parameters");
    }
}

const glm::vec3& PerspectiveCamera::position() const {
    return m_position;
}

float PerspectiveCamera::yawDegrees() const {
    return m_yawDegrees;
}

float PerspectiveCamera::pitchDegrees() const {
    return m_pitchDegrees;
}

float PerspectiveCamera::fieldOfViewDegrees() const {
    return m_fieldOfViewDegrees;
}

float PerspectiveCamera::nearClip() const {
    return m_nearClip;
}

float PerspectiveCamera::farClip() const {
    return m_farClip;
}

float PerspectiveCamera::aspectRatio() const {
    return m_aspectRatio;
}

glm::vec3 PerspectiveCamera::forward() const {
    const float yaw = glm::radians(m_yawDegrees);
    const float pitch = glm::radians(m_pitchDegrees);
    return glm::normalize(glm::vec3(
        glm::cos(yaw) * glm::cos(pitch),
        glm::sin(pitch),
        glm::sin(yaw) * glm::cos(pitch)));
}

glm::vec3 PerspectiveCamera::right() const {
    return glm::normalize(glm::cross(forward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 PerspectiveCamera::up() const {
    return glm::normalize(glm::cross(right(), forward()));
}

void PerspectiveCamera::setPosition(const glm::vec3& position) {
    m_position = position;
    m_viewDirty = true;
}

void PerspectiveCamera::translate(const glm::vec3& offset) {
    m_position += offset;
    m_viewDirty = true;
}

void PerspectiveCamera::setOrientation(float yawDegrees, float pitchDegrees) {
    m_yawDegrees = yawDegrees;
    m_pitchDegrees = pitchDegrees;
    m_viewDirty = true;
}

void PerspectiveCamera::setAspectRatio(float aspectRatio) {
    if (aspectRatio <= 0.0f) {
        return;
    }
    m_aspectRatio = aspectRatio;
    m_projectionDirty = true;
}

void PerspectiveCamera::setPerspective(
    float verticalFieldOfViewDegrees,
    float nearClip,
    float farClip) {
    if (verticalFieldOfViewDegrees <= 0.0f ||
        verticalFieldOfViewDegrees >= 180.0f ||
        nearClip <= 0.0f ||
        farClip <= nearClip) {
        throw std::invalid_argument("Invalid perspective camera parameters");
    }

    m_fieldOfViewDegrees = verticalFieldOfViewDegrees;
    m_nearClip = nearClip;
    m_farClip = farClip;
    m_projectionDirty = true;
}

const glm::mat4& PerspectiveCamera::viewMatrix() const {
    if (m_viewDirty) {
        updateViewMatrix();
    }
    return m_viewMatrix;
}

const glm::mat4& PerspectiveCamera::projectionMatrix() const {
    if (m_projectionDirty) {
        updateProjectionMatrix();
    }
    return m_projectionMatrix;
}

void PerspectiveCamera::updateViewMatrix() const {
    m_viewMatrix = glm::lookAt(m_position, m_position + forward(), up());
    m_viewDirty = false;
}

void PerspectiveCamera::updateProjectionMatrix() const {
    m_projectionMatrix = glm::perspective(
        glm::radians(m_fieldOfViewDegrees),
        m_aspectRatio,
        m_nearClip,
        m_farClip);
    m_projectionDirty = false;
}

} // namespace Engine
