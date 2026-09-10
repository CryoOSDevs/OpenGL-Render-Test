#include "ViewportPanel.h"

#include <glad/glad.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stdexcept>

ViewportPanel::ViewportPanel() {
shader_ = Shader::FromFile("shaders/mesh.vert", "shaders/mesh.frag");
}

ViewportPanel::~ViewportPanel() {
Shutdown();
}

bool ViewportPanel::Render(std::vector<SceneObject>& objects,
                      int selectedIndex,
                      const OrbitCamera& camera,
                      const std::vector<SceneLight>& lights,
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

RenderScene(camera, objects, lights);

const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
ImGui::Image((ImTextureID)(uintptr_t)framebuffer_.TextureID(), contentSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

if (selectedIndex >= 0 && selectedIndex < static_cast<int>(objects.size())) {
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, contentSize.x, contentSize.y);

    glm::mat4 model = objects[selectedIndex].WorldMatrix();
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
        objects[selectedIndex].transform.position = glm::vec3(translation[0], translation[1], translation[2]);
        objects[selectedIndex].transform.rotation = glm::quat(glm::radians(glm::vec3(rotation[0], rotation[1], rotation[2])));
        objects[selectedIndex].transform.scale = glm::vec3(scale[0], scale[1], scale[2]);
    }

    if (isUsingGizmo != nullptr) {
        *isUsingGizmo = ImGuizmo::IsUsing();
    }
    if (isHoveredByGizmo != nullptr) {
        *isHoveredByGizmo = ImGuizmo::IsOver();
    }
}

ImGui::End();
return false;
}

void ViewportPanel::Shutdown() {
shader_.Destroy();
framebuffer_.Destroy();
hovered_ = false;
focused_ = false;
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

void ViewportPanel::RenderScene(const OrbitCamera& camera, const std::vector<SceneObject>& objects, const std::vector<SceneLight>& lights) const {
const glm::ivec2 size = framebuffer_.Size();
if (size.x <= 0 || size.y <= 0) {
    return;
}

framebuffer_.Bind();
glViewport(0, 0, size.x, size.y);
glEnable(GL_DEPTH_TEST);
glClearColor(0.12f, 0.13f, 0.17f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

glUseProgram(shader_.GetId());
const glm::mat4 view = camera.ViewMatrix();
const glm::mat4 projection = camera.ProjectionMatrix(static_cast<float>(size.x) / static_cast<float>(size.y));
shader_.SetMat4("uView", view);
shader_.SetMat4("uProjection", projection);
shader_.SetVec3("uCameraPos", camera.Position());

glm::vec3 ambient(0.18f, 0.2f, 0.25f);
glm::vec3 dirLight(0.6f, 0.75f, 1.0f);
shader_.SetVec3("uAmbient", ambient);
shader_.SetVec3("uLightDir", glm::normalize(glm::vec3(-0.7f, -1.0f, -0.5f)));
shader_.SetVec3("uLightColor", dirLight);

if (!lights.empty()) {
    const SceneLight& light = lights.front();
    shader_.SetVec3("uLightDir", glm::normalize(light.position - glm::vec3(0.0f, 0.0f, 0.0f)));
    shader_.SetVec3("uLightColor", light.color * light.intensity);
}

SceneObject ground;
ground.name = "Ground";
ground.SetPlaneMesh();
ground.transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
ground.transform.scale = glm::vec3(12.0f, 1.0f, 12.0f);
ground.color = glm::vec4(0.67f, 0.68f, 0.72f, 1.0f);
ground.EnsureUpload();
shader_.SetMat4("uModel", ground.WorldMatrix());
shader_.SetVec4("uColor", ground.color);
ground.Draw();

for (const auto& object : objects) {
    object.EnsureUpload();
    shader_.SetMat4("uModel", object.WorldMatrix());
    shader_.SetVec4("uColor", object.color);
    object.Draw();
}

glUseProgram(0);
framebuffer_.Unbind();
}
