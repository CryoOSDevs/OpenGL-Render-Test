#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include "../Camera.h"
#include "../Framebuffer.h"
#include "../SceneObject.h"
#include "../Shader.h"

struct SceneLight {
    glm::vec3 position{0.0f, 2.0f, 3.0f};
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

class ViewportPanel {
public:
    ViewportPanel();
    ~ViewportPanel();

    bool Render(std::vector<SceneObject>& objects,
                int selectedIndex,
                const OrbitCamera& camera,
                const std::vector<SceneLight>& lights,
                ImGuizmo::OPERATION operation,
                ImGuizmo::MODE mode,
                bool isLocalMode,
                bool* isUsingGizmo = nullptr,
                bool* isHoveredByGizmo = nullptr);

    void Shutdown();
    bool IsHovered() const;
    bool IsFocused() const;
    unsigned int TextureID() const;
    glm::ivec2 Size() const;

private:
    void RenderScene(const OrbitCamera& camera, const std::vector<SceneObject>& objects, const std::vector<SceneLight>& lights) const;

    Framebuffer framebuffer_;
    Shader shader_;
    bool hovered_ = false;
    bool focused_ = false;
};
