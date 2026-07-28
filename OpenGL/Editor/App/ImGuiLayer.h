#pragma once

struct GLFWwindow;

namespace Editor {

class ImGuiLayer final {
public:
    void init(GLFWwindow* window);
    void beginFrame();
    void endFrame();
    void shutdown();

    [[nodiscard]] bool wantCaptureMouse() const;
    [[nodiscard]] bool wantCaptureKeyboard() const;

private:
    bool m_initialized = false;
};

} // namespace Editor
