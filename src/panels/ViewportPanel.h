#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include "../Camera.h"
#include "../Framebuffer.h"
#include "../SceneObject.h"
#include "../Shader.h"

class ViewportPanel {
public:
    ViewportPanel();
    ~ViewportPanel();

    bool Render(SceneObject& object,
                const OrbitCamera& camera,
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
    void BuildCube();
    void RenderScene(const OrbitCamera& camera, const SceneObject& object) const;

    Framebuffer framebuffer_;
    Shader shader_;
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    bool hovered_ = false;
    bool focused_ = false;
};
