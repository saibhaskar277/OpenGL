#include "Engine/Core/Window.h"
#include "Engine/Renderer/DirectionalShadowPass.h"
#include "Engine/Renderer/FlyCameraController.h"
#include "Engine/Renderer/Lighting/LightingEnvironment.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/PerspectiveCamera.h"
#include "Engine/Renderer/PrimitiveFactory.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Scene/Transform.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>

int main() {
    try {
        Engine::Window window(800, 600, "Rendering Engine");
        window.setCursorCaptured(true);

        Engine::Mesh cubeMesh = Engine::PrimitiveFactory::createCube();

        Engine::Shader shader(
            "Assets/Shaders/Lit.vert",
            "Assets/Shaders/Lit.frag");
        Engine::DirectionalShadowPass shadowPass(
            2048,
            "Assets/Shaders/ShadowDepth.vert",
            "Assets/Shaders/ShadowDepth.frag",
            6.0f);

        Engine::Transform cubeTransform;
        cubeTransform.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        cubeTransform.setRotationDegrees(glm::vec3(-15.0f, 30.0f, 0.0f));

        Engine::Transform floorTransform;
        floorTransform.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
        floorTransform.setScale(glm::vec3(8.0f, 0.1f, 8.0f));

        const Engine::Material cubeMaterial{
            glm::vec3(0.95f, 0.32f, 0.08f),
            glm::vec3(0.8f),
            64.0f
        };
        const Engine::Material floorMaterial{
            glm::vec3(0.18f, 0.20f, 0.24f),
            glm::vec3(0.25f),
            24.0f
        };

        Engine::PerspectiveCamera camera(45.0f, window.aspectRatio(), 0.1f, 100.0f);
        camera.setPosition(glm::vec3(0.0f, 1.2f, 4.5f));
        camera.setOrientation(-90.0f, -10.0f);
        Engine::FlyCameraController cameraController;

        Engine::LightingEnvironment lighting;
        lighting.directionalLight().direction = glm::vec3(-0.4f, -1.0f, -0.25f);
        lighting.directionalLight().color = glm::vec3(0.72f, 0.82f, 1.0f);
        lighting.directionalLight().intensity = 0.75f;

        Engine::PointLight pointLight;
        pointLight.position = glm::vec3(2.0f, 1.8f, 1.5f);
        pointLight.color = glm::vec3(1.0f, 0.32f, 0.08f);
        pointLight.intensity = 5.0f;
        lighting.addPointLight(pointLight);

        lighting.spotLight().color = glm::vec3(0.55f, 0.72f, 1.0f);
        lighting.spotLight().intensity = 2.0f;

        lighting.hemisphereLight().skyColor = glm::vec3(0.28f, 0.42f, 0.7f);
        lighting.hemisphereLight().groundColor = glm::vec3(0.20f, 0.08f, 0.035f);
        lighting.hemisphereLight().intensity = 0.35f;
        lighting.hemisphereLight().bounceColor = glm::vec3(1.0f, 0.24f, 0.05f);
        lighting.hemisphereLight().bounceStrength = 0.55f;
        lighting.hemisphereLight().floorHeight = -0.95f;

        Engine::Renderer::setClearColor(0.025f, 0.035f, 0.065f, 1.0f);
        Engine::Renderer::setDepthTesting(true);

        using Clock = std::chrono::steady_clock;
        auto previousFrame = Clock::now();

        while (!window.shouldClose()) {
            const auto currentFrame = Clock::now();
            const float deltaTime = std::min(
                std::chrono::duration<float>(currentFrame - previousFrame).count(),
                0.1f);
            previousFrame = currentFrame;

            if (window.isKeyPressed(GLFW_KEY_ESCAPE)) {
                window.requestClose();
            }

            camera.setAspectRatio(window.aspectRatio());
            cameraController.update(window, camera, deltaTime);
            cubeTransform.rotateDegrees(glm::vec3(
                12.0f * deltaTime,
                30.0f * deltaTime,
                0.0f));

            lighting.spotLight().position = camera.position();
            lighting.spotLight().direction = camera.forward();

            shadowPass.begin(
                lighting.directionalLight().direction,
                glm::vec3(0.0f, -0.25f, 0.0f));
            shadowPass.draw(cubeMesh, cubeTransform.matrix());
            shadowPass.draw(cubeMesh, floorTransform.matrix());
            shadowPass.end(
                window.framebufferWidth(),
                window.framebufferHeight());

            Engine::Renderer::clear();
            shader.bind();
            shader.setMat4("uView", camera.viewMatrix());
            shader.setMat4("uProjection", camera.projectionMatrix());
            shadowPass.bindForLighting(shader, 0);

            shader.setMat4("uModel", cubeTransform.matrix());
            lighting.apply(shader, camera.position(), cubeMaterial);
            Engine::Renderer::draw(cubeMesh, shader);

            shader.setMat4("uModel", floorTransform.matrix());
            lighting.apply(shader, camera.position(), floorMaterial);
            Engine::Renderer::draw(cubeMesh, shader);

            window.swapBuffers();
            window.pollEvents();
        }
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
