#include "Editor/App/EditorApp.h"

#include "Editor/App/EditorLayer.h"
#include "Editor/App/ImGuiLayer.h"
#include "Editor/Viewport/EditorCameraController.h"
#include "Editor/Viewport/Picking.h"
#include "Engine/Assets/AssetManager.h"
#include "Engine/Core/Window.h"
#include "Engine/Renderer/Shadows/LightShadowSystem.h"
#include "Engine/Camera/PerspectiveCamera.h"
#include "Engine/Renderer/RHI/Renderer.h"
#include "Engine/Renderer/RHI/Shader.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneRenderer.h"
#include "Engine/Scene/SceneSerializer.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>

namespace Editor {

int EditorApp::run() {
    try {
        Engine::Window window(1280, 720, "OpenGL Rendering Editor");
        window.setCursorCaptured(false);

        ImGuiLayer imgui;
        imgui.init(window.nativeHandle());

        Engine::AssetManager assets("Assets");
        assets.refresh();

        Engine::Scene scene;
        try {
            Engine::SceneSerializer(scene).load("Assets/Scenes/default.json");
        } catch (const std::exception& error) {
            std::cerr << "Failed to load default scene, creating new one: "
                      << error.what() << '\n';
            scene.createDefaultScene();
        }

        Engine::Shader litShader(
            "Assets/Shaders/Lit.vert",
            "Assets/Shaders/Lit.frag");
        Engine::LightShadowSystem shadows;
        Engine::SceneRenderer sceneRenderer;

        Engine::PerspectiveCamera camera(45.0f, window.aspectRatio(), 0.1f, 100.0f);
        camera.setPosition(glm::vec3(0.0f, 1.2f, 4.5f));
        camera.setOrientation(-90.0f, -10.0f);

        EditorCameraController editorCamera;
        editorCamera.resetPivot(glm::vec3(0.0f, 0.0f, 0.0f));

        EditorLayer editorLayer;
        editorLayer.setScenePath("Assets/Scenes/default.json");
        editorLayer.markClean();

        Engine::Renderer::setClearColor(0.025f, 0.035f, 0.065f, 1.0f);
        Engine::Renderer::setDepthTesting(true);

        using Clock = std::chrono::steady_clock;
        auto previousFrame = Clock::now();
        bool mouseWasPressed = false;

        while (!window.shouldClose()) {
            const auto currentFrame = Clock::now();
            const float deltaTime = std::min(
                std::chrono::duration<float>(currentFrame - previousFrame).count(),
                0.1f);
            previousFrame = currentFrame;

            window.pollEvents();
            imgui.beginFrame();

            if (window.isKeyPressed(GLFW_KEY_ESCAPE)) {
                window.requestClose();
            }

            camera.setAspectRatio(window.aspectRatio());
            const bool captureMouse = imgui.wantCaptureMouse();
            const bool captureKeyboard = imgui.wantCaptureKeyboard();
            const bool altDown =
                glfwGetKey(window.nativeHandle(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
                glfwGetKey(window.nativeHandle(), GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
            const bool rightMouse =
                glfwGetMouseButton(window.nativeHandle(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
            const bool leftMouse =
                glfwGetMouseButton(window.nativeHandle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

            editorCamera.update(
                window,
                camera,
                scene,
                deltaTime,
                captureMouse,
                captureKeyboard);

            // Click-to-select when not flying / alt-navigating / dragging gizmo.
            if (!captureMouse &&
                !rightMouse &&
                !altDown &&
                leftMouse &&
                !mouseWasPressed &&
                !editorLayer.gizmo().isUsing() &&
                !editorLayer.gizmo().isOver()) {
                double mouseX = 0.0;
                double mouseY = 0.0;
                window.cursorPosition(mouseX, mouseY);
                const Engine::Entity hit = pickEntity(
                    scene,
                    camera,
                    static_cast<float>(mouseX),
                    static_cast<float>(mouseY),
                    static_cast<float>(window.framebufferWidth()),
                    static_cast<float>(window.framebufferHeight()));
                if (hit != Engine::NullEntity) {
                    scene.setSelectedEntity(hit);
                }
            }
            mouseWasPressed = leftMouse;

            sceneRenderer.render(
                scene,
                assets,
                shadows,
                litShader,
                camera,
                window.framebufferWidth(),
                window.framebufferHeight());

            editorLayer.onImGuiRender(
                window,
                scene,
                assets,
                camera,
                editorCamera,
                deltaTime,
                static_cast<float>(window.framebufferWidth()),
                static_cast<float>(window.framebufferHeight()));
            imgui.endFrame();
            window.swapBuffers();
        }

        imgui.shutdown();
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }
}

} // namespace Editor
