#include "Engine/Core/Window.h"
#include "Engine/Renderer/FlyCameraController.h"
#include "Engine/Renderer/LightShadowSystem.h"
#include "Engine/Renderer/Lighting/LightingEnvironment.h"
#include "Engine/Renderer/Material.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Model.h"
#include "Engine/Renderer/PerspectiveCamera.h"
#include "Engine/Renderer/PrimitiveFactory.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Scene/Transform.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>

namespace {

void drawMeshLit(
    Engine::Shader& shader,
    Engine::LightingEnvironment& lighting,
    const Engine::PerspectiveCamera& camera,
    const Engine::Mesh& mesh,
    const Engine::Material& material,
    const glm::mat4& modelMatrix) {
    shader.setMat4("uModel", modelMatrix);
    lighting.apply(shader, camera.position(), material);
    material.bindMaps(shader, 1);
    Engine::Renderer::draw(mesh, shader);
}

void drawModelLit(
    Engine::Shader& shader,
    Engine::LightingEnvironment& lighting,
    const Engine::PerspectiveCamera& camera,
    const Engine::Model& model,
    const glm::mat4& modelMatrix) {
    for (const Engine::ModelPart& part : model.parts()) {
        drawMeshLit(
            shader,
            lighting,
            camera,
            part.mesh,
            part.material,
            modelMatrix);
    }
}

} // namespace

int main() {
    try {
        Engine::Window window(800, 600, "Rendering Engine");
        window.setCursorCaptured(true);

        Engine::Model cubeModel = Engine::Model::loadObj("Assets/Models/cube.obj");
        Engine::Mesh floorMesh = Engine::PrimitiveFactory::createCube();

        auto checkerAlbedo = Engine::Texture::createCheckerboard(256, 32, 210, 55);
        auto flatNormal = Engine::Texture::createFlatNormal();
        auto softSpecular = Engine::Texture::createSolid(
            90,
            90,
            90,
            255,
            Engine::TextureColorSpace::Linear);

        for (Engine::ModelPart& part : cubeModel.parts()) {
            if (!part.material.albedoMap) {
                part.material.albedoMap = checkerAlbedo;
            }
            if (!part.material.normalMap) {
                part.material.normalMap = flatNormal;
            }
            if (!part.material.specularMap) {
                part.material.specularMap = softSpecular;
            }
            part.material.albedo = glm::vec3(1.0f, 0.55f, 0.2f);
            part.material.shininess = 64.0f;
        }

        Engine::Material floorMaterial;
        floorMaterial.albedo = glm::vec3(0.75f);
        floorMaterial.specularColor = glm::vec3(0.2f);
        floorMaterial.shininess = 24.0f;
        floorMaterial.albedoMap = Engine::Texture::createCheckerboard(256, 48, 48, 160);
        floorMaterial.normalMap = flatNormal;
        floorMaterial.aoMap = Engine::Texture::createSolid(
            220,
            220,
            220,
            255,
            Engine::TextureColorSpace::Linear);

        Engine::Shader shader(
            "Assets/Shaders/Lit.vert",
            "Assets/Shaders/Lit.frag");
        Engine::LightShadowSystem shadows;

        Engine::Transform cubeTransform;
        cubeTransform.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        cubeTransform.setRotationDegrees(glm::vec3(-15.0f, 30.0f, 0.0f));

        Engine::Transform floorTransform;
        floorTransform.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));
        floorTransform.setScale(glm::vec3(8.0f, 0.1f, 8.0f));

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

            // One submit: shadows for directional + spot + every point light.
            shadows.render(
                lighting,
                glm::vec3(0.0f, -0.25f, 0.0f),
                window.framebufferWidth(),
                window.framebufferHeight(),
                [&](Engine::LightShadowSystem::CasterDrawer& draw) {
                    draw(cubeModel, cubeTransform.matrix());
                    draw(floorMesh, floorTransform.matrix());
                });

            Engine::Renderer::clear();
            shader.bind();
            shader.setMat4("uView", camera.viewMatrix());
            shader.setMat4("uProjection", camera.projectionMatrix());
            shadows.bindForLighting(shader);

            drawModelLit(
                shader,
                lighting,
                camera,
                cubeModel,
                cubeTransform.matrix());
            drawMeshLit(
                shader,
                lighting,
                camera,
                floorMesh,
                floorMaterial,
                floorTransform.matrix());

            window.swapBuffers();
            window.pollEvents();
        }
    } catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
