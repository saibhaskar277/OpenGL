# OpenGL Learning Project

Small C++ / OpenGL **rendering engine** with a decoupled **ImGui editor**.

## Layout

```text
OpenGL/
  Engine/                 Core only (NO ImGui)
  Editor/                 ImGui editor
  Sandbox/src/main.cpp
  Assets/
  Dependencies/           ALL third-party libs (single source)
    include/              GLM, GLFW headers, glad, nlohmann/json, stb
    lib/                  glfw3.lib (Windows)
    imgui/                Dear ImGui + ImGuizmo
    JoltPhysics/          Jolt Physics sources
CMakeLists.txt            Only build entry
```

## Build (only path)

```bash
cmake -B build
cmake --build build --config Release
```

Visual Studio: open the generated **`build/OpenGL.sln`**, set startup project **OpenGLApp**, build/run.

```bash
./build/Release/OpenGLApp.exe
```

There is no separate hand-made `.sln` under `OpenGL/` — CMake is the only solution.

### Linux packages (example)

```bash
sudo apt install build-essential cmake libgl1-mesa-dev libglfw3-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev
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
- **Drag texture** from Project → Inspector map slots
- **File → New / Save / Save As / Open Scene**
- **Hierarchy** — Add Cube / Physics Cube / Light / Delete
- **Inspector** — transform, material, lights, rigid body + collider
- **Physics → Simulate** — run Jolt simulation
- **Performance overlay** (top-left) — FPS, frame time, counts

## Architecture

```text
Editor (ImGui)     →  Scene (ECS) + PhysicsSystem
PhysicsSystem      →  IPhysicsWorld
JoltPhysicsWorld   →  only place that includes Jolt
SceneRenderer      →  Scene + AssetManager
SceneSerializer    →  JSON ↔ Scene
```

Engine never includes ImGui. Renderer/Scene do not include Jolt.
