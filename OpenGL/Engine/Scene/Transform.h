#pragma once

#include <glm/glm.hpp>

namespace Engine {

class Transform final {
public:
    [[nodiscard]] const glm::vec3& position() const;
    [[nodiscard]] const glm::vec3& rotationDegrees() const;
    [[nodiscard]] const glm::vec3& scale() const;

    void setPosition(const glm::vec3& position);
    void setRotationDegrees(const glm::vec3& rotation);
    void setScale(const glm::vec3& scale);

    void translate(const glm::vec3& offset);
    void rotateDegrees(const glm::vec3& offset);

    [[nodiscard]] glm::mat4 matrix() const;

private:
    glm::vec3 m_position{0.0f};
    glm::vec3 m_rotationDegrees{0.0f};
    glm::vec3 m_scale{1.0f};
};

} // namespace Engine
