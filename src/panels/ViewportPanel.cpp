#include "ViewportPanel.h"

#include <glad/gl.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <stdexcept>

namespace {
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

const Vertex kCubeVertices[] = {
    // Front
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    // Back
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    // Left
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    // Right
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    // Top
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    // Bottom
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
};
}  // namespace

ViewportPanel::ViewportPanel() {
    shader_ = Shader::FromFile("shaders/mesh.vert", "shaders/mesh.frag");
    BuildCube();
}

ViewportPanel::~ViewportPanel() {
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
    }
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
    }
}

bool ViewportPanel::Render(SceneObject& object,
                          const OrbitCamera& camera,
                          ImGuizmo::OPERATION operation,
                          ImGuizmo::MODE mode,
                          bool isLocalMode,
                          bool* isUsingGizmo,
                          bool* isHoveredByGizmo) {
    ImGui::Begin("Viewport");
    hovered_ = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
    focused_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    const ImVec2 contentSize = ImGui::GetContentRegionAvail();
    if (contentSize.x > 1.0f && contentSize.y > 1.0f) {
        framebuffer_.Resize(static_cast<int>(contentSize.x), static_cast<int>(contentSize.y));
    }

    RenderScene(camera, object);

    const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImGui::Image((ImTextureID)(uintptr_t)framebuffer_.TextureID(), contentSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, contentSize.x, contentSize.y);

    glm::mat4 model = object.WorldMatrix();
    glm::mat4 view = camera.ViewMatrix();
    glm::mat4 projection = camera.ProjectionMatrix(contentSize.x / std::max(1.0f, contentSize.y));

    bool changed = false;
    if (contentSize.x > 1.0f && contentSize.y > 1.0f) {
        changed = ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), operation,
                                      isLocalMode ? ImGuizmo::LOCAL : ImGuizmo::WORLD, glm::value_ptr(model));
    }

    if (changed) {
        float translation[3];
        float rotation[3];
        float scale[3];
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);
        object.transform.position = glm::vec3(translation[0], translation[1], translation[2]);
        object.transform.rotation = glm::quat(glm::radians(glm::vec3(rotation[0], rotation[1], rotation[2])));
        object.transform.scale = glm::vec3(scale[0], scale[1], scale[2]);
    }

    if (isUsingGizmo != nullptr) {
        *isUsingGizmo = ImGuizmo::IsUsing();
    }
    if (isHoveredByGizmo != nullptr) {
        *isHoveredByGizmo = ImGuizmo::IsOver();
    }

    ImGui::End();
    return changed;
}

bool ViewportPanel::IsHovered() const {
    return hovered_;
}

bool ViewportPanel::IsFocused() const {
    return focused_;
}

unsigned int ViewportPanel::TextureID() const {
    return framebuffer_.TextureID();
}

glm::ivec2 ViewportPanel::Size() const {
    return framebuffer_.Size();
}

void ViewportPanel::BuildCube() {
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kCubeVertices), kCubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

    glBindVertexArray(0);
}

void ViewportPanel::RenderScene(const OrbitCamera& camera, const SceneObject& object) const {
    const glm::ivec2 size = framebuffer_.Size();
    if (size.x <= 0 || size.y <= 0) {
        return;
    }

    framebuffer_.Bind();
    glViewport(0, 0, size.x, size.y);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_.GetId());
    const glm::mat4 model = object.WorldMatrix();
    const glm::mat4 view = camera.ViewMatrix();
    const glm::mat4 projection = camera.ProjectionMatrix(static_cast<float>(size.x) / static_cast<float>(size.y));
    shader_.SetMat4("uModel", model);
    shader_.SetMat4("uView", view);
    shader_.SetMat4("uProjection", projection);
    shader_.SetVec4("uColor", object.color);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    framebuffer_.Unbind();
    glUseProgram(0);
}
