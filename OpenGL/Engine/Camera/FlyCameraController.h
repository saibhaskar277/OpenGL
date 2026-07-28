#pragma once

namespace Engine {

class PerspectiveCamera;
class Window;

class FlyCameraController final {
public:
    FlyCameraController(float movementSpeed = 3.0f, float mouseSensitivity = 0.1f);

    void update(Window& window, PerspectiveCamera& camera, float deltaTime);
    void resetMouseTracking();

    void setMovementSpeed(float movementSpeed);
    void setMouseSensitivity(float mouseSensitivity);
    [[nodiscard]] float movementSpeed() const;
    [[nodiscard]] float mouseSensitivity() const;

private:
    float m_movementSpeed;
    float m_mouseSensitivity;
    double m_previousMouseX = 0.0;
    double m_previousMouseY = 0.0;
    bool m_firstMouseUpdate = true;
};

} // namespace Engine
