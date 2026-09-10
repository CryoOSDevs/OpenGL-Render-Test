#include "ViewportPanel.h"

#include <glad/glad.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>
#include <stdexcept>

namespace {
bool IntersectRaySphere(const glm::vec3& rayOrigin,
                   const glm::vec3& rayDirection,
                   const glm::vec3& center,
                   float radius,
                   float* distance = nullptr) {
const glm::vec3 offset = rayOrigin - center;
const float b = glm::dot(offset, rayDirection);
const float c = glm::dot(offset, offset) - radius * radius;
const float discriminant = b * b - c;
if (discriminant < 0.0f) {
    return false;
}
const float sqrtDisc = std::sqrt(discriminant);
const float t0 = -b - sqrtDisc;
const float t1 = -b + sqrtDisc;
const float best = (t0 > 0.0f) ? t0 : ((t1 > 0.0f) ? t1 : -1.0f);
if (best <= 0.0f) {
    return false;
}
if (distance != nullptr) {
    *distance = best;
}
return true;
}

glm::vec3 UnprojectMouse(const glm::vec2& mouseNdc,
                     const glm::mat4& view,
                     const glm::mat4& projection,
                     const glm::vec2& viewportSize) {
const glm::vec4 clipNear(mouseNdc.x, mouseNdc.y, -1.0f, 1.0f);
const glm::vec4 clipFar(mouseNdc.x, mouseNdc.y, 1.0f, 1.0f);
const glm::mat4 invViewProj = glm::inverse(projection * view);
const glm::vec4 worldNear = invViewProj * clipNear;
const glm::vec4 worldFar = invViewProj * clipFar;
const glm::vec3 nearPoint = glm::vec3(worldNear.x, worldNear.y, worldNear.z) / worldNear.w;
const glm::vec3 farPoint = glm::vec3(worldFar.x, worldFar.y, worldFar.z) / worldFar.w;
return glm::normalize(farPoint - nearPoint);
}
}  // namespace

ViewportPanel::ViewportPanel() {
shader_ = Shader::FromFile("shaders/mesh.vert", "shaders/mesh.frag");
}

ViewportPanel::~ViewportPanel() {
Shutdown();
}

bool ViewportPanel::Render(std::vector<SceneObject>& objects,
                      int* selectedIndex,
                      const OrbitCamera& camera,
                      const std::vector<SceneLight>& lights,
                      ImGuizmo::OPERATION operation,
                      ImGuizmo::MODE mode,
                      bool isLocalMode,
                      bool* isUsingGizmo,
                      bool* isHoveredByGizmo) {
(void)mode;
ImGui::SetNextWindowDockID(ImGui::GetID("LauraEditorDockspace"), ImGuiCond_Once);
ImGui::Begin("Viewport");
hovered_ = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
focused_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

const ImVec2 contentSize = ImGui::GetContentRegionAvail();
if (contentSize.x > 1.0f && contentSize.y > 1.0f) {
    framebuffer_.Resize(static_cast<int>(contentSize.x), static_cast<int>(contentSize.y));
}

const int selectedObjectIndex = (selectedIndex != nullptr && *selectedIndex >= 0 && *selectedIndex < static_cast<int>(objects.size())) ? *selectedIndex : -1;
RenderScene(camera, objects, lights, selectedObjectIndex);

const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
ImGui::Image((ImTextureID)(uintptr_t)framebuffer_.TextureID(), contentSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

if (selectedIndex != nullptr && *selectedIndex >= 0 && *selectedIndex < static_cast<int>(objects.size())) {
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, contentSize.x, contentSize.y);

    glm::mat4 model = objects[*selectedIndex].WorldMatrix();
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
        objects[*selectedIndex].transform.position = glm::vec3(translation[0], translation[1], translation[2]);
        objects[*selectedIndex].transform.rotation = glm::quat(glm::radians(glm::vec3(rotation[0], rotation[1], rotation[2])));
        objects[*selectedIndex].transform.scale = glm::vec3(scale[0], scale[1], scale[2]);
    }

    if (isUsingGizmo != nullptr) {
        *isUsingGizmo = ImGuizmo::IsUsing();
    }
    if (isHoveredByGizmo != nullptr) {
        *isHoveredByGizmo = ImGuizmo::IsOver();
    }
}

if (selectedIndex != nullptr && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) &&
    ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
    const ImVec2 mousePos = ImGui::GetMousePos();
    const ImVec2 windowMin = ImGui::GetWindowPos();
    const ImVec2 mouseLocal = ImVec2(mousePos.x - windowMin.x - ImGui::GetWindowContentRegionMin().x,
                                      mousePos.y - windowMin.y - ImGui::GetWindowContentRegionMin().y);
    const glm::vec2 mouseNdc(
        (mouseLocal.x / std::max(1.0f, contentSize.x)) * 2.0f - 1.0f,
        1.0f - (mouseLocal.y / std::max(1.0f, contentSize.y)) * 2.0f);

    const glm::mat4 view = camera.ViewMatrix();
    const glm::mat4 projection = camera.ProjectionMatrix(contentSize.x / std::max(1.0f, contentSize.y));
    const glm::vec4 nearClip(mouseNdc.x, mouseNdc.y, -1.0f, 1.0f);
    const glm::vec4 farClip(mouseNdc.x, mouseNdc.y, 1.0f, 1.0f);
    const glm::vec4 nearWorld = glm::inverse(projection * view) * nearClip;
    const glm::vec4 farWorld = glm::inverse(projection * view) * farClip;
    const glm::vec3 rayOrigin = glm::vec3(nearWorld.x, nearWorld.y, nearWorld.z) / nearWorld.w;
    const glm::vec3 rayDirection = glm::normalize((glm::vec3(farWorld.x, farWorld.y, farWorld.z) / farWorld.w) - rayOrigin);

    int picked = -1;
    float closest = std::numeric_limits<float>::max();
    for (int i = 0; i < static_cast<int>(objects.size()); ++i) {
        const auto& object = objects[i];
        if (object.name == "Ground") {
            continue;
        }
        const glm::vec3 center = object.WorldCenter();
        const float radius = object.WorldRadius();
        float hitDistance = 0.0f;
        if (IntersectRaySphere(rayOrigin, rayDirection, center, radius, &hitDistance) && hitDistance < closest) {
            closest = hitDistance;
            picked = i;
        }
    }

    if (picked >= 0) {
        *selectedIndex = picked;
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

void ViewportPanel::RenderScene(const OrbitCamera& camera, std::vector<SceneObject>& objects, const std::vector<SceneLight>& lights, int selectedIndex) const {
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

glm::vec3 ambient(0.22f, 0.24f, 0.30f);
glm::vec3 dirLightColor(0.92f, 0.88f, 0.80f);
glm::vec3 dirLightDirection = glm::normalize(glm::vec3(-0.8f, -1.0f, -0.6f));
shader_.SetVec3("uAmbient", ambient);
shader_.SetVec3("uDirLightDir", dirLightDirection);
shader_.SetVec3("uDirLightColor", dirLightColor);

glm::vec3 pointLightPos(2.0f, 3.5f, 3.5f);
glm::vec3 pointLightColor(1.0f, 0.92f, 0.86f);
if (!lights.empty()) {
    const SceneLight& light = lights.front();
    pointLightPos = light.position;
    pointLightColor = light.color * light.intensity;
}
shader_.SetVec3("uPointLightPos", pointLightPos);
shader_.SetVec3("uPointLightColor", pointLightColor);

SceneObject ground;
ground.name = "Ground";
ground.SetPlaneMesh();
ground.transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
ground.transform.scale = glm::vec3(12.0f, 1.0f, 12.0f);
ground.color = glm::vec4(0.67f, 0.68f, 0.72f, 1.0f);
ground.EnsureUpload();
shader_.SetMat4("uModel", ground.WorldMatrix());
shader_.SetVec4("uColor", ground.color);
shader_.SetFloat("uSelected", 0.0f);
ground.Draw();

for (int i = 0; i < static_cast<int>(objects.size()); ++i) {
    auto& object = objects[i];
    object.EnsureUpload();
    shader_.SetMat4("uModel", object.WorldMatrix());
    shader_.SetVec4("uColor", object.color);
    shader_.SetFloat("uSelected", (selectedIndex == i) ? 1.0f : 0.0f);
    object.Draw();
}

glUseProgram(0);
framebuffer_.Unbind();
}
