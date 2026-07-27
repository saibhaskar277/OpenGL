#include "Engine/Renderer/TangentSpace.h"

#include <glm/glm.hpp>

#include <cmath>

namespace Engine {

void computeTangents(
    std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices) {
    if (vertices.empty() || indices.size() < 3) {
        return;
    }

    std::vector<glm::vec3> accumulated(vertices.size(), glm::vec3(0.0f));

    for (std::size_t i = 0; i + 2 < indices.size(); i += 3) {
        const unsigned int i0 = indices[i];
        const unsigned int i1 = indices[i + 1];
        const unsigned int i2 = indices[i + 2];
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
            continue;
        }

        Vertex& v0 = vertices[i0];
        Vertex& v1 = vertices[i1];
        Vertex& v2 = vertices[i2];

        const glm::vec3 edge1 = v1.position - v0.position;
        const glm::vec3 edge2 = v2.position - v0.position;
        const glm::vec2 deltaUv1 = v1.texCoord - v0.texCoord;
        const glm::vec2 deltaUv2 = v2.texCoord - v0.texCoord;

        const float denominator =
            deltaUv1.x * deltaUv2.y - deltaUv2.x * deltaUv1.y;
        if (std::abs(denominator) < 0.000001f) {
            continue;
        }

        const float f = 1.0f / denominator;
        const glm::vec3 tangent = f * (deltaUv2.y * edge1 - deltaUv1.y * edge2);
        accumulated[i0] += tangent;
        accumulated[i1] += tangent;
        accumulated[i2] += tangent;
    }

    for (std::size_t i = 0; i < vertices.size(); ++i) {
        glm::vec3 tangent = accumulated[i];
        const glm::vec3& normal = vertices[i].normal;
        tangent = tangent - normal * glm::dot(normal, tangent);
        if (glm::dot(tangent, tangent) < 0.000001f) {
            const glm::vec3 helper =
                std::abs(normal.y) < 0.999f
                ? glm::vec3(0.0f, 1.0f, 0.0f)
                : glm::vec3(1.0f, 0.0f, 0.0f);
            tangent = glm::normalize(glm::cross(helper, normal));
        } else {
            tangent = glm::normalize(tangent);
        }
        vertices[i].tangent = tangent;
    }
}

void computeTangents(std::vector<Vertex>& vertices) {
    std::vector<unsigned int> indices(vertices.size());
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        indices[i] = static_cast<unsigned int>(i);
    }
    computeTangents(vertices, indices);
}

} // namespace Engine
