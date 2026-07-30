cmake_minimum_required(VERSION 3.16)

project(OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL")
set(DEPS_ROOT "${APP_SOURCE_ROOT}/Dependencies")

find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# All third-party code lives under OpenGL/Dependencies (single source).
# ---------------------------------------------------------------------------

# GLFW — prebuilt lib on Windows; system package elsewhere
if(WIN32)
    add_library(glfw STATIC IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${DEPS_ROOT}/lib/glfw3.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${DEPS_ROOT}/include"
    )
    set(OPENGL_GLFW_TARGET glfw)
else()
    find_package(glfw3 REQUIRED)
    if(TARGET glfw)
        set(OPENGL_GLFW_TARGET glfw)
    else()
        set(OPENGL_GLFW_TARGET glfw3::glfw)
    endif()
endif()

# Dear ImGui + ImGuizmo (vendored)
add_library(imgui STATIC
    "${DEPS_ROOT}/imgui/imgui.cpp"
    "${DEPS_ROOT}/imgui/imgui_draw.cpp"
    "${DEPS_ROOT}/imgui/imgui_tables.cpp"
    "${DEPS_ROOT}/imgui/imgui_widgets.cpp"
    "${DEPS_ROOT}/imgui/backends/imgui_impl_glfw.cpp"
    "${DEPS_ROOT}/imgui/backends/imgui_impl_opengl3.cpp"
    "${DEPS_ROOT}/imgui/ImGuizmo/ImGuizmo.cpp"
)
target_include_directories(imgui PUBLIC
    "${DEPS_ROOT}/imgui"
    "${DEPS_ROOT}/imgui/backends"
    "${DEPS_ROOT}/imgui/ImGuizmo"
    "${DEPS_ROOT}/include"
)
target_link_libraries(imgui PUBLIC ${OPENGL_GLFW_TARGET} OpenGL::GL)

# nlohmann/json (header-only, vendored)
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE "${DEPS_ROOT}/include")

# Jolt Physics (vendored sources)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
set(ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_ROOT}/JoltPhysics/Build" "${CMAKE_BINARY_DIR}/_deps/jolt-build")

# ---------------------------------------------------------------------------
# App sources
# ---------------------------------------------------------------------------
set(ENGINE_SOURCES
    "${APP_SOURCE_ROOT}/Engine/Core/glad.c"
    "${APP_SOURCE_ROOT}/Engine/Core/Window.cpp"
    "${APP_SOURCE_ROOT}/Engine/Assets/AssetManager.cpp"
    "${APP_SOURCE_ROOT}/Engine/Camera/FlyCameraController.cpp"
    "${APP_SOURCE_ROOT}/Engine/Camera/PerspectiveCamera.cpp"
    "${APP_SOURCE_ROOT}/Engine/Physics/PhysicsMath.cpp"
    "${APP_SOURCE_ROOT}/Engine/Physics/PhysicsSystem.cpp"
    "${APP_SOURCE_ROOT}/Engine/Physics/ColliderFactory.cpp"
    "${APP_SOURCE_ROOT}/Engine/Physics/Jolt/JoltPhysicsWorld.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Lighting/LightingEnvironment.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Mesh/Mesh.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Mesh/Model.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Mesh/PrimitiveFactory.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Mesh/TangentSpace.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/RHI/IndexBuffer.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/RHI/Material.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/RHI/Renderer.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/RHI/Shader.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/RHI/Texture.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/RHI/VertexArray.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/RHI/VertexBuffer.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Shadows/CubemapShadowMap.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Shadows/DirectionalShadowPass.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Shadows/LightShadowSystem.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Shadows/ShadowMap.cpp"
    "${APP_SOURCE_ROOT}/Engine/Scene/Transform.cpp"
    "${APP_SOURCE_ROOT}/Engine/Scene/Registry.cpp"
    "${APP_SOURCE_ROOT}/Engine/Scene/Scene.cpp"
    "${APP_SOURCE_ROOT}/Engine/Scene/SceneRenderer.cpp"
    "${APP_SOURCE_ROOT}/Engine/Scene/SceneSerializer.cpp"
)

set(EDITOR_SOURCES
    "${APP_SOURCE_ROOT}/Editor/App/EditorApp.cpp"
    "${APP_SOURCE_ROOT}/Editor/App/EditorLayer.cpp"
    "${APP_SOURCE_ROOT}/Editor/App/ImGuiLayer.cpp"
    "${APP_SOURCE_ROOT}/Editor/Viewport/EditorCameraController.cpp"
    "${APP_SOURCE_ROOT}/Editor/Viewport/LightDebugOverlay.cpp"
    "${APP_SOURCE_ROOT}/Editor/Viewport/ColliderDebugOverlay.cpp"
    "${APP_SOURCE_ROOT}/Editor/Viewport/Picking.cpp"
    "${APP_SOURCE_ROOT}/Editor/Viewport/ViewportGizmo.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/CameraPanel.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/HierarchyPanel.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/InspectorPanel.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/ContentBrowserPanel.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/PerformancePanel.cpp"
    "${APP_SOURCE_ROOT}/Sandbox/src/main.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "${APP_SOURCE_ROOT}/Engine/*.h"
    "${APP_SOURCE_ROOT}/Engine/*.hpp"
)
file(GLOB_RECURSE EDITOR_HEADERS CONFIGURE_DEPENDS
    "${APP_SOURCE_ROOT}/Editor/*.h"
    "${APP_SOURCE_ROOT}/Editor/*.hpp"
)

set(APP_ALL_SOURCES ${ENGINE_SOURCES} ${EDITOR_SOURCES} ${ENGINE_HEADERS} ${EDITOR_HEADERS})

add_executable(OpenGLApp ${APP_ALL_SOURCES})

source_group(TREE "${APP_SOURCE_ROOT}" FILES ${APP_ALL_SOURCES})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(OpenGLApp PROPERTIES FOLDER "App")
set_target_properties(imgui PROPERTIES FOLDER "Dependencies")
set_target_properties(Jolt PROPERTIES FOLDER "Dependencies")

target_include_directories(OpenGLApp PRIVATE
    "${APP_SOURCE_ROOT}"
    "${DEPS_ROOT}/include"
    "${DEPS_ROOT}/JoltPhysics"
)

target_link_libraries(OpenGLApp PRIVATE
    ${OPENGL_GLFW_TARGET}
    OpenGL::GL
    imgui
    nlohmann_json
    Jolt
)

if(WIN32)
    target_link_libraries(OpenGLApp PRIVATE user32 gdi32 shell32)
endif()

if(APPLE)
    target_compile_definitions(OpenGLApp PRIVATE GL_SILENCE_DEPRECATION)
endif()

add_custom_command(TARGET OpenGLApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${APP_SOURCE_ROOT}/Assets"
        "$<TARGET_FILE_DIR:OpenGLApp>/Assets"
    COMMENT "Copying Assets next to OpenGLApp"
)

set_target_properties(OpenGLApp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:OpenGLApp>"
)

# Make F5 run the app, not CMake's ALL_BUILD helper target.
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT OpenGLApp)

install(TARGETS OpenGLApp RUNTIME DESTINATION bin)
install(DIRECTORY "${APP_SOURCE_ROOT}/Assets/" DESTINATION bin/Assets)
