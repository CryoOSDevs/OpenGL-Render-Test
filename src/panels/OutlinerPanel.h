#pragma once

#include <vector>

#include "../SceneObject.h"

class OutlinerPanel {
public:
    void Render(std::vector<SceneObject>& objects, int* selectedIndex);
};
