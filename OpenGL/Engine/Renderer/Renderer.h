#pragma once

namespace Engine {

class Shader;
class VertexArray;
class Mesh;

class Renderer final {
public:
    Renderer() = delete;

    static void setClearColor(float red, float green, float blue, float alpha);
    static void setDepthTesting(bool enabled);
    static void clear();
    static void drawTriangles(
        const VertexArray& vertexArray,
        const Shader& shader,
        int vertexCount);
    static void draw(const Mesh& mesh, const Shader& shader);
};

} // namespace Engine
