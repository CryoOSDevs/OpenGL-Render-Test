#include "Shader.h"

#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace {
unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::string shaderType = type == GL_VERTEX_SHADER ? "vertex" : "fragment";
        throw std::runtime_error("Shader compile error (" + shaderType + "): " + std::string(log));
    }
    return shader;
}

unsigned int LinkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        throw std::runtime_error("Shader link error: " + std::string(log));
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

std::string ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}  // namespace

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    const unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
    const unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
    id_ = LinkProgram(vertexShader, fragmentShader);
}

Shader::~Shader() {
    Destroy();
}

Shader::Shader(Shader&& other) noexcept : id_(other.id_) {
    other.id_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (id_ != 0) {
            glDeleteProgram(id_);
        }
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

void Shader::Use() const {
    if (id_ == 0) {
        throw std::runtime_error("Attempted to use an invalid shader program");
    }
    glUseProgram(id_);
}

void Shader::SetMat4(const char* name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(id_, name), 1, GL_FALSE, &value[0][0]);
}

void Shader::SetInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(id_, name), value);
}

void Shader::SetFloat(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(id_, name), value);
}

void Shader::SetVec3(const char* name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(id_, name), 1, &value[0]);
}

void Shader::SetVec4(const char* name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(id_, name), 1, &value[0]);
}

unsigned int Shader::GetId() const {
    return id_;
}

void Shader::Destroy() {
    if (id_ != 0) {
        glDeleteProgram(id_);
        id_ = 0;
    }
}

Shader Shader::FromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    return Shader(ReadFile(vertexPath), ReadFile(fragmentPath));
}
