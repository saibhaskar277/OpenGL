#include "Engine/Scene/Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine {

const glm::vec3& Transform::position() const {
    return m_position;
}

const glm::vec3& Transform::rotationDegrees() const {
    return m_rotationDegrees;
}

const glm::vec3& Transform::scale() const {
    return m_scale;
}

void Transform::setPosition(const glm::vec3& position) {
    m_position = position;
}

void Transform::setRotationDegrees(const glm::vec3& rotation) {
    m_rotationDegrees = rotation;
}

void Transform::setScale(const glm::vec3& scale) {
    m_scale = scale;
}

void Transform::translate(const glm::vec3& offset) {
    m_position += offset;
}

void Transform::rotateDegrees(const glm::vec3& offset) {
    m_rotationDegrees += offset;
}

glm::mat4 Transform::matrix() const {
    const glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
    const glm::quat orientation(glm::radians(m_rotationDegrees));
    const glm::mat4 rotation = glm::mat4_cast(orientation);
    const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_scale);
    return translation * rotation * scaleMatrix;
}

} // namespace Engine
