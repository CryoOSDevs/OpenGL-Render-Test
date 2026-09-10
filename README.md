# Laura Editor

Minimal UE5-style editor prototype built with C++17, CMake, GLFW, GLAD, ImGui docking, and ImGuizmo.

## Build

```bash
cmake -B build
cmake --build build
```

## Usage

- Right mouse drag: orbit camera
- Middle mouse drag: pan camera
- Mouse wheel: zoom camera
- W: translate gizmo
- E: rotate gizmo
- R: scale gizmo
- L: toggle local/world transform space

The viewport renders a single cube object; the Details panel edits its transform and color, and the Outliner exposes selection.
