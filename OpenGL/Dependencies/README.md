# Dependencies

Single place for all third-party libraries. The app and CMake use **only** this folder (no FetchContent downloads).

| Path | What |
|------|------|
| `include/` | GLM, GLFW headers, glad/KHR, nlohmann/json, stb |
| `lib/glfw3.lib` | GLFW (Windows prebuilt) |
| `imgui/` | Dear ImGui (docking) + backends + ImGuizmo |
| `JoltPhysics/` | Jolt Physics sources (`Jolt/` + `Build/`) |

On Linux, install system GLFW (`libglfw3-dev`); headers under `include/` are still used for GLM/json/stb.
