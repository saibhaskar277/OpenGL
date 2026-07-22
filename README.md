# OpenGL Learning Project

Personal C++ / OpenGL sandbox for learning graphics programming — a small engine layer, a sandbox app, shaders, and study notes.

## Layout

```text
OpenGL/
  Engine/          Engine source (window, renderer, camera, lighting, shadows)
  Sandbox/         App entry / demos
  Assets/Shaders/  GLSL shaders
  Docs/            Learning notes (C++ + OpenGL/GLSL)
  Dependencies/    Headers + GLFW lib (GLM, glad, GLFW)
  OpenGL.sln       Visual Studio solution
```

## Docs

Learning material lives under [`OpenGL/Docs/`](OpenGL/Docs/):

- [C++ Fundamentals](OpenGL/Docs/CPP_Fundamentals.md) — language theory and practice
- [OpenGL Engine Guide](OpenGL/Docs/OpenGL_Engine_Guide.md) — OpenGL, GLSL, and how this project uses them

## Build

1. Open `OpenGL/OpenGL.sln` in Visual Studio 2022.
2. Build **Debug | x64** (or Release).
3. Run the Sandbox target.

Requires a Windows OpenGL-capable GPU driver and the MSVC v143 toolset.

## What’s tracked

Only source, shaders, docs, project files, and dependencies. Build outputs, Visual Studio cache (`.vs`), and intermediate files are ignored.
