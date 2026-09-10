#pragma once

#include <glm/glm.hpp>
#include <string>

class Shader {
public:
    Shader() = default;
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    void Use() const;
    void SetMat4(const char* name, const glm::mat4& value) const;
    void SetInt(const char* name, int value) const;
    void SetFloat(const char* name, float value) const;
    void SetVec3(const char* name, const glm::vec3& value) const;
    void SetVec4(const char* name, const glm::vec4& value) const;
    unsigned int GetId() const;
    void Destroy();

    static Shader FromFile(const std::string& vertexPath, const std::string& fragmentPath);

private:
    unsigned int id_ = 0;
};
