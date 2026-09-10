#include "OutlinerPanel.h"

#include <imgui.h>

void OutlinerPanel::Render(std::vector<SceneObject>& objects, int* selectedIndex) {
    ImGui::Begin("Outliner");
    for (size_t i = 0; i < objects.size(); ++i) {
        const bool isSelected = selectedIndex != nullptr && *selectedIndex == static_cast<int>(i);
        if (ImGui::Selectable(objects[i].name.c_str(), isSelected)) {
            if (selectedIndex != nullptr) {
                *selectedIndex = static_cast<int>(i);
            }
        }
    }
    ImGui::End();
}
