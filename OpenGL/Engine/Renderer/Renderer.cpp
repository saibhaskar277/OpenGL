#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/VertexArray.h"

#include <glad/glad.h>

namespace Engine {

void Renderer::setClearColor(float red, float green, float blue, float alpha) {
    glClearColor(red, green, blue, alpha);
}

void Renderer::setDepthTesting(bool enabled) {
    if (enabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void Renderer::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::drawTriangles(
    const VertexArray& vertexArray,
    const Shader& shader,
    int vertexCount) {
    shader.bind();
    vertexArray.bind();
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void Renderer::draw(const Mesh& mesh, const Shader& shader) {
    drawTriangles(mesh.vertexArray(), shader, mesh.vertexCount());
}

} // namespace Engine
