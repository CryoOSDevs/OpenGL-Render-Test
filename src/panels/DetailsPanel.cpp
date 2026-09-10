#include "DetailsPanel.h"

#include <glm/gtc/quaternion.hpp>
#include <imgui.h>

namespace {
glm::vec3 EulerDegrees(const glm::quat& rotation) {
    return glm::degrees(glm::eulerAngles(rotation));
}
}  // namespace

bool DetailsPanel::Render(SceneObject& object) {
    bool changed = false;
    ImGui::SetNextWindowDockID(ImGui::GetID("LauraEditorDockspace"), ImGuiCond_Once);
    ImGui::Begin("Details");

    ImGui::Text("%s", object.name.c_str());

    glm::vec3 position = object.transform.position;
    if (ImGui::InputFloat3("Position", &position.x)) {
        object.transform.position = position;
        changed = true;
    }

    glm::vec3 rotationEuler = EulerDegrees(object.transform.rotation);
    if (ImGui::InputFloat3("Rotation (deg)", &rotationEuler.x)) {
        object.transform.rotation = glm::quat(glm::radians(rotationEuler));
        changed = true;
    }

    glm::vec3 scale = object.transform.scale;
    if (ImGui::InputFloat3("Scale", &scale.x)) {
        object.transform.scale = scale;
        changed = true;
    }

    glm::vec4 color = object.color;
    if (ImGui::ColorEdit4("Color", &color.x)) {
        object.color = color;
        changed = true;
    }

    ImGui::End();
    return changed;
}
