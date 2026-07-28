#pragma once

#include <glm/fwd.hpp>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace Engine {

class Shader final {
public:
    Shader(
        const std::filesystem::path& vertexPath,
        const std::filesystem::path& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void bind() const;
    static void unbind();

    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setFloat3(const std::string& name, float x, float y, float z);
    void setFloat4(const std::string& name, float x, float y, float z, float w);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat4(const std::string& name, const glm::mat4& value);

private:
    static std::string readSource(const std::filesystem::path& path);
    static unsigned int compile(unsigned int type, const std::string& source, const std::filesystem::path& path);
    static unsigned int link(unsigned int vertexShader, unsigned int fragmentShader);
    int uniformLocation(const std::string& name);
    void release();

    unsigned int m_program = 0;
    std::unordered_map<std::string, int> m_uniformLocations;
};

} // namespace Engine
