#include "Engine/Core/Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace Engine {

Window::Window(int width, int height, const std::string& title) {
    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (m_window == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glfwGetFramebufferSize(m_window, &m_framebufferWidth, &m_framebufferHeight);
    glViewport(0, 0, m_framebufferWidth, m_framebufferHeight);
}

Window::~Window() {
    if (m_window != nullptr) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

int Window::framebufferWidth() const {
    return m_framebufferWidth;
}

int Window::framebufferHeight() const {
    return m_framebufferHeight;
}

float Window::aspectRatio() const {
    if (m_framebufferHeight == 0) {
        return 1.0f;
    }
    return static_cast<float>(m_framebufferWidth) /
        static_cast<float>(m_framebufferHeight);
}

void Window::requestClose() {
    glfwSetWindowShouldClose(m_window, GLFW_TRUE);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

void Window::swapBuffers() const {
    glfwSwapBuffers(m_window);
}

void Window::cursorPosition(double& x, double& y) const {
    glfwGetCursorPos(m_window, &x, &y);
}

void Window::setCursorCaptured(bool captured) {
    glfwSetInputMode(
        m_window,
        GLFW_CURSOR,
        captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* owner = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (owner != nullptr) {
        owner->m_framebufferWidth = width;
        owner->m_framebufferHeight = height;
    }
    glViewport(0, 0, width, height);
}

} // namespace Engine
