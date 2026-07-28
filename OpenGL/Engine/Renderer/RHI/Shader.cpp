#include "Engine/Renderer/RHI/Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Engine {

Shader::Shader(
    const std::filesystem::path& vertexPath,
    const std::filesystem::path& fragmentPath) {
    const std::string vertexSource = readSource(vertexPath);
    const std::string fragmentSource = readSource(fragmentPath);

    const unsigned int vertexShader = compile(GL_VERTEX_SHADER, vertexSource, vertexPath);
    unsigned int fragmentShader = 0;

    try {
        fragmentShader = compile(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);
        m_program = link(vertexShader, fragmentShader);
    } catch (...) {
        glDeleteShader(vertexShader);
        if (fragmentShader != 0) {
            glDeleteShader(fragmentShader);
        }
        throw;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    release();
}

Shader::Shader(Shader&& other) noexcept
    : m_program(std::exchange(other.m_program, 0)),
      m_uniformLocations(std::move(other.m_uniformLocations)) {
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        release();
        m_program = std::exchange(other.m_program, 0);
        m_uniformLocations = std::move(other.m_uniformLocations);
    }
    return *this;
}

void Shader::bind() const {
    glUseProgram(m_program);
}

void Shader::unbind() {
    glUseProgram(0);
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(uniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(uniformLocation(name), value);
}

void Shader::setFloat3(const std::string& name, float x, float y, float z) {
    glUniform3f(uniformLocation(name), x, y, z);
}

void Shader::setFloat4(const std::string& name, float x, float y, float z, float w) {
    glUniform4f(uniformLocation(name), x, y, z, w);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(uniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(uniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

std::string Shader::readSource(const std::filesystem::path& path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    if (!stream) {
        throw std::runtime_error("Unable to open shader file: " + path.string());
    }

    std::ostringstream source;
    source << stream.rdbuf();
    return source.str();
}

unsigned int Shader::compile(
    unsigned int type,
    const std::string& source,
    const std::filesystem::path& path) {
    const unsigned int shader = glCreateShader(type);
    const char* sourceData = source.c_str();
    glShaderSource(shader, 1, &sourceData, nullptr);
    glCompileShader(shader);

    int success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_TRUE) {
        return shader;
    }

    int logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<std::size_t>(logLength > 0 ? logLength : 1));
    glGetShaderInfoLog(shader, logLength, nullptr, log.data());
    glDeleteShader(shader);

    throw std::runtime_error(
        "Shader compilation failed (" + path.string() + "):\n" + log.data());
}

unsigned int Shader::link(unsigned int vertexShader, unsigned int fragmentShader) {
    const unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_TRUE) {
        return program;
    }

    int logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<std::size_t>(logLength > 0 ? logLength : 1));
    glGetProgramInfoLog(program, logLength, nullptr, log.data());
    glDeleteProgram(program);

    throw std::runtime_error("Shader program linking failed:\n" + std::string(log.data()));
}

int Shader::uniformLocation(const std::string& name) {
    const auto cached = m_uniformLocations.find(name);
    if (cached != m_uniformLocations.end()) {
        return cached->second;
    }

    const int location = glGetUniformLocation(m_program, name.c_str());
    m_uniformLocations.emplace(name, location);
    return location;
}

void Shader::release() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

} // namespace Engine
