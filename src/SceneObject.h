#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

struct Transform {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
};

struct MeshVertex {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
};

class SceneObject {
public:
    SceneObject() = default;
    ~SceneObject();

    SceneObject(const SceneObject& other);
    SceneObject& operator=(const SceneObject& other);
    SceneObject(SceneObject&& other) noexcept;
    SceneObject& operator=(SceneObject&& other) noexcept;

    std::string name = "Cube";
    Transform transform;
    glm::vec4 color{0.78f, 0.63f, 0.41f, 1.0f};

    void SetCubeMesh();
    void SetPlaneMesh();
    void SetMesh(const std::vector<MeshVertex>& vertices, const std::vector<unsigned int>& indices);
    void Draw() const;
    void EnsureUpload();
    void ReleaseGLResources();

    glm::mat4 WorldMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, transform.position);
        model *= glm::mat4_cast(transform.rotation);
        model = glm::scale(model, transform.scale);
        return model;
    }

    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;

private:
    void CopyFrom(const SceneObject& other);
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLsizei indexCount_ = 0;
    bool uploaded_ = false;
};

inline SceneObject::~SceneObject() {
    ReleaseGLResources();
}

inline SceneObject::SceneObject(const SceneObject& other) {
    CopyFrom(other);
}

inline SceneObject& SceneObject::operator=(const SceneObject& other) {
    if (this != &other) {
        ReleaseGLResources();
        CopyFrom(other);
    }
    return *this;
}

inline SceneObject::SceneObject(SceneObject&& other) noexcept {
    CopyFrom(other);
    other.ReleaseGLResources();
    other.vertices.clear();
    other.indices.clear();
}

inline SceneObject& SceneObject::operator=(SceneObject&& other) noexcept {
    if (this != &other) {
        ReleaseGLResources();
        CopyFrom(other);
        other.ReleaseGLResources();
        other.vertices.clear();
        other.indices.clear();
    }
    return *this;
}

inline void SceneObject::CopyFrom(const SceneObject& other) {
    name = other.name;
    transform = other.transform;
    color = other.color;
    vertices = other.vertices;
    indices = other.indices;
    vao_ = 0;
    vbo_ = 0;
    ebo_ = 0;
    indexCount_ = 0;
    uploaded_ = false;
}

inline void SceneObject::ReleaseGLResources() {
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    uploaded_ = false;
    indexCount_ = 0;
}

inline void SceneObject::SetCubeMesh() {
    static const std::vector<MeshVertex> cubeVertices = {
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}}, {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}}, {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}}, {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}}, {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}}, {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}}, {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}}, {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}}, {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}}, {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}, {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}
    };
    vertices = cubeVertices;
    indices.resize(vertices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = static_cast<unsigned int>(i);
    }
    uploaded_ = false;
}

inline void SceneObject::SetPlaneMesh() {
    vertices = {
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}}, {{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}}
    };
    indices = {0, 1, 2, 3, 4, 5};
    uploaded_ = false;
}

inline void SceneObject::SetMesh(const std::vector<MeshVertex>& newVertices, const std::vector<unsigned int>& newIndices) {
    vertices = newVertices;
    indices = newIndices;
    uploaded_ = false;
}

inline void SceneObject::EnsureUpload() {
    if (uploaded_ || vertices.empty()) {
        return;
    }

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(MeshVertex)), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MeshVertex), reinterpret_cast<void*>(offsetof(MeshVertex, normal)));

    if (!indices.empty()) {
        glGenBuffers(1, &ebo_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
        indexCount_ = static_cast<GLsizei>(indices.size());
    } else {
        indexCount_ = static_cast<GLsizei>(vertices.size());
    }

    glBindVertexArray(0);
    uploaded_ = true;
}

inline void SceneObject::Draw() const {
    if (vao_ == 0) {
        return;
    }
    glBindVertexArray(vao_);
    if (ebo_ != 0) {
        glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, indexCount_);
    }
    glBindVertexArray(0);
}
