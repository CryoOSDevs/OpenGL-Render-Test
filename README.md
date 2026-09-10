# Laura Editor

Laura Editor is a compact UE5-inspired prototype editor built in C++17 with CMake, GLFW, GLAD, ImGui docking, and ImGuizmo. It focuses on a polished, editor-style workspace: a docked viewport, transform gizmo, details panel, outliner, orbit camera controls, lighting, scene spawning, and lightweight mesh importing.

## Features

- Dockable editor layout with viewport, outliner, details, and toolbar panels
- 3D viewport with offscreen framebuffer rendering and ImGuizmo transform overlay
- Orbit / pan / zoom camera controls
- Transform gizmo hotkeys: W = translate, E = rotate, R = scale, L = local/world toggle
- Panel-driven editing for object position, rotation, scale, and color
- Gray platform-style editor theme with a cleaner default layout
- Scene lighting and a simple ground plane
- Add Cube and Add Light actions
- Asset browser for .obj, .glb, and .gltf imports into the scene
- Core profile OpenGL 3.3 rendering

## Requirements

- CMake 3.16+
- C/C++ compiler
- Python 3

The project uses FetchContent to pull in GLFW, GLM, ImGui (docking), ImGuizmo, and tinygltf automatically. The GLAD Python package is installed during configure if needed.

## Build

```bash
cmake -B build
cmake --build build
```

To run the editor:

```bash
./build/LauraEditor
```

## Controls

- Right mouse drag: orbit camera
- Middle mouse drag: pan camera
- Mouse wheel: zoom camera
- W: translate gizmo
- E: rotate gizmo
- R: scale gizmo
- L: local/world transform space toggle
- Click an object in the outliner or viewport to select it
- The Details panel edits the selected object’s transform and color live

## Asset import

Place asset files into an `assets/` folder in the project root, then refresh the Asset Browser panel. Supported formats:

- .obj
- .glb
- .gltf

Imported meshes are added to the scene and appear in the outliner.

## Notes

This is intentionally a lightweight editor prototype rather than a production engine. It is designed to stay compact and easy to understand while still offering the editor feel, transform workflow, lighting, and asset-import basics expected from a UE5-style viewport tool.
