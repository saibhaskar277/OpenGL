#pragma once

#include <string>

struct GLFWwindow;

namespace Engine {

class Window final {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    [[nodiscard]] bool shouldClose() const;
    [[nodiscard]] bool isKeyPressed(int key) const;
    [[nodiscard]] int framebufferWidth() const;
    [[nodiscard]] int framebufferHeight() const;
    [[nodiscard]] float aspectRatio() const;
    void requestClose();
    void pollEvents() const;
    void swapBuffers() const;
    void cursorPosition(double& x, double& y) const;
    void setCursorCaptured(bool captured);
    [[nodiscard]] GLFWwindow* nativeHandle() const;

private:
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window = nullptr;
    int m_framebufferWidth = 0;
    int m_framebufferHeight = 0;
};

} // namespace Engine
