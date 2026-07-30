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
    Panels/               Hierarchy, Inspector, Project, Camera
  Sandbox/src/main.cpp    Thin entry → EditorApp
  Assets/                 Scenes, Textures, Models, Shaders
CMakeLists.txt
```

## Features

- ECS scene (entities + components)
- Scene JSON as project assets (New / Open / Save / Save As)
- Unity-style **Project** panel (recursive `Assets/` tree)
- Drag-drop models into Hierarchy; drag textures onto material slots
- Lighting: directional / point / spot / hemisphere
- Shadows for directional, spot, and point lights
- Textures: albedo, normal, specular, emissive, AO
- OBJ/MTL model loading
- ImGui editor: picking, gizmos, light debug markers, camera panel

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

- **RMB + WASD / Space / Shift** — fly camera
- **Alt + LMB** — orbit around pivot
- **Alt + MMB** (or **Alt + Shift + LMB**) — pan
- **Alt + RMB** / **mouse wheel** — zoom (dolly)
- **F** — focus selected entity
- **LMB** — select mesh/light; drag gizmo to move
- **Project** — browse `Assets/` tree; Refresh; Create Folder / Create Scene
- **Drag model** from Project → Hierarchy — creates entity with Material texture slots
- **Drag texture** from Project → Inspector map slots (or onto Hierarchy with a mesh selected for albedo)
- **Double-click scene** in Project — open that scene
- **File → New / Save / Save As / Open Scene** — scene session (title shows `*` when dirty)
- **Hierarchy** — select entities; right-click → Add Cube / Light / Delete
- **Inspector** — transform, material colors, exposed texture maps, lights
- **Performance overlay** (top-left) — FPS, frame time, entity/mesh/light counts

Place files under `Assets/Textures`, `Assets/Models`, or `Assets/Scenes`, then **Refresh** in Project. Nested folders are supported.

## Architecture

```text
Editor (ImGui)  →  mutates Scene (ECS)
SceneRenderer   →  reads Scene + AssetManager → shadows + lit draw
SceneSerializer →  JSON ↔ Scene
```

Engine never includes ImGui.
