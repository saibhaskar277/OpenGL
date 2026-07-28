# OpenGL Learning Project

Small C++ / OpenGL **rendering engine** with a decoupled **ImGui editor**.

## Layout

```text
OpenGL/
  Engine/                 Core only (NO ImGui)
    Core/                 Window, glad
    Camera/               PerspectiveCamera, FlyCameraController
    Assets/               AssetManager
    Scene/                ECS, Scene, SceneRenderer, JSON serializer
    Renderer/
      RHI/                buffers, shader, texture, material, draw API
      Mesh/               Mesh, Model, primitives, tangents
      Lighting/           directional / point / spot / hemisphere
      Shadows/            shadow maps + LightShadowSystem
  Editor/                 ImGui editor (depends on Engine)
    App/                  EditorApp, EditorLayer, ImGuiLayer
    Viewport/             picking, gizmo, light debug markers
    Panels/               Hierarchy, Inspector, Content Browser, Camera
  Sandbox/src/main.cpp    Thin entry → EditorApp
  Assets/                 Scenes, Textures, Models, Shaders
CMakeLists.txt
```

## Features

- ECS scene (entities + components)
- Scene JSON save/load
- Local asset browser for textures/models
- Lighting: directional / point / spot / hemisphere
- Shadows for directional, spot, and point lights
- Textures: albedo, normal, specular, emissive, AO
- OBJ/MTL model loading
- ImGui editor: add cubes, edit transforms/materials/lights, assign textures

## Build (CMake — recommended)

```bash
cmake -B build
cmake --build build --config Release
```

Run from the output folder (so `Assets/` resolves):

```bash
./build/Release/OpenGLApp.exe
```

## Build (Visual Studio)

1. Open `OpenGL/OpenGL.sln`
2. Set **Debug | x64** (or Release)
3. Build and run (`OpenGL.exe` under `OpenGL/x64/...`)

ImGui and nlohmann/json are vendored under `OpenGL/Dependencies/` for the VS project.

### Dependencies (auto-fetched)

- GLFW (if not installed)
- Dear ImGui (docking)
- nlohmann/json

### Linux packages (example)

```bash
sudo apt install build-essential cmake libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev
```

## Editor usage

- **Hold RMB** — fly camera look/move
- **Hierarchy** — select entities; right-click → Add Cube / Delete
- **Inspector** — transform, material colors, texture slots, lights
- **Content Browser** — refresh and browse `Assets/Textures` + `Assets/Models`
- **File → Save/Load Scene** — `Assets/Scenes/default.json`
- **Edit → Add Cube** — spawn a cube entity

Drop texture files into `Assets/Textures`, hit **Refresh Assets**, then assign them in the Inspector.

## Architecture

```text
Editor (ImGui)  →  mutates Scene (ECS)
SceneRenderer   →  reads Scene + AssetManager → shadows + lit draw
SceneSerializer →  JSON ↔ Scene
```

Engine never includes ImGui.
