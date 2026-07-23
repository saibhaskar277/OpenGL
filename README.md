# OpenGL Learning Project

Personal C++ / OpenGL sandbox for learning graphics programming — a small engine layer, a sandbox app, shaders, and study notes.

## Layout

```text
OpenGL/
  Engine/          Engine source (window, renderer, camera, lighting, shadows)
  Sandbox/         App entry / demos
  Assets/Shaders/  GLSL shaders
  Docs/            Learning notes (C++ + OpenGL/GLSL)
  Dependencies/    Headers (GLM, glad, GLFW) + Windows glfw3.lib for VS
  OpenGL.sln       Visual Studio solution (Windows convenience)
CMakeLists.txt     Cross-platform build (Windows / Linux / macOS)
```

## Docs

Learning material lives under [`OpenGL/Docs/`](OpenGL/Docs/):

- [C++ Fundamentals](OpenGL/Docs/CPP_Fundamentals.md) — language theory and practice
- [OpenGL Engine Guide](OpenGL/Docs/OpenGL_Engine_Guide.md) — OpenGL, GLSL, and how this project uses them

## Requirements

- C++17 compiler
- CMake 3.16+
- OpenGL 3.3+ capable GPU/driver
- GLFW: system package if installed, otherwise CMake fetches GLFW 3.4 automatically (needs network on first configure)

### Linux packages (example)

```bash
# Debian / Ubuntu
sudo apt install build-essential cmake libgl1-mesa-dev libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev

# Fedora
sudo dnf install cmake gcc-c++ mesa-libGL-devel libX11-devel libXrandr-devel libXi-devel libXcursor-devel libXinerama-devel
```

Optional: `libglfw3-dev` / `glfw-devel` if you prefer a system GLFW instead of FetchContent.

### macOS

```bash
xcode-select --install
brew install cmake
```

## Build with CMake (recommended for Linux / macOS)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run (from the folder that contains the binary, so `Assets/` resolves):

```bash
# Linux / macOS
./build/OpenGLApp

# Windows (example)
.\build\Release\OpenGLApp.exe
```

Visual Studio via CMake:

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Build with Visual Studio (Windows)

1. Open `OpenGL/OpenGL.sln` in Visual Studio 2022.
2. Build **Debug | x64** (or Release).
3. Run the project.

Uses the vendored `Dependencies/lib/glfw3.lib`. MSVC toolset v143 required.

## What’s tracked

Source, shaders, docs, VS project files, and dependency headers. Build outputs, CMake `build/`, and Visual Studio cache are ignored. Generated CMake solutions are not committed — each platform generates its own.
