cmake_minimum_required(VERSION 3.16)

project(OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL")

include(FetchContent)

# ---------------------------------------------------------------------------
# GLFW
# ---------------------------------------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

find_package(glfw3 CONFIG QUIET)
if(NOT glfw3_FOUND AND NOT TARGET glfw)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PC_GLFW QUIET glfw3)
    endif()
endif()

if(TARGET glfw)
    set(OPENGL_GLFW_TARGET glfw)
elseif(TARGET glfw3::glfw)
    set(OPENGL_GLFW_TARGET glfw3::glfw)
elseif(PC_GLFW_FOUND)
    add_library(glfw_pkginterface INTERFACE)
    target_include_directories(glfw_pkginterface INTERFACE ${PC_GLFW_INCLUDE_DIRS})
    target_link_libraries(glfw_pkginterface INTERFACE ${PC_GLFW_LIBRARIES})
    set(OPENGL_GLFW_TARGET glfw_pkginterface)
else()
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)
    set(OPENGL_GLFW_TARGET glfw)
endif()

find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# nlohmann/json
# ---------------------------------------------------------------------------
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

# ---------------------------------------------------------------------------
# Dear ImGui (docking)
# ---------------------------------------------------------------------------
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
    "${APP_SOURCE_ROOT}/Dependencies/imgui/ImGuizmo/ImGuizmo.cpp"
)
target_include_directories(imgui PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
    "${APP_SOURCE_ROOT}/Dependencies/imgui/ImGuizmo"
)
target_link_libraries(imgui PUBLIC ${OPENGL_GLFW_TARGET} OpenGL::GL)

# ---------------------------------------------------------------------------
# Engine sources
# ---------------------------------------------------------------------------
set(ENGINE_SOURCES
    "${APP_SOURCE_ROOT}/Engine/Core/glad.c"
    "${APP_SOURCE_ROOT}/Engine/Core/Window.cpp"
    "${APP_SOURCE_ROOT}/Engine/Assets/AssetManager.cpp"
    "${APP_SOURCE_ROOT}/Engine/Camera/FlyCameraController.cpp"
    "${APP_SOURCE_ROOT}/Engine/Camera/PerspectiveCamera.cpp"
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
    "${APP_SOURCE_ROOT}/Editor/Viewport/LightDebugOverlay.cpp"
    "${APP_SOURCE_ROOT}/Editor/Viewport/Picking.cpp"
    "${APP_SOURCE_ROOT}/Editor/Viewport/ViewportGizmo.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/CameraPanel.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/HierarchyPanel.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/InspectorPanel.cpp"
    "${APP_SOURCE_ROOT}/Editor/Panels/ContentBrowserPanel.cpp"
    "${APP_SOURCE_ROOT}/Sandbox/src/main.cpp"
)

add_executable(OpenGLApp ${ENGINE_SOURCES} ${EDITOR_SOURCES})

target_include_directories(OpenGLApp PRIVATE
    "${APP_SOURCE_ROOT}"
    "${APP_SOURCE_ROOT}/Dependencies/include"
)

target_link_libraries(OpenGLApp PRIVATE
    ${OPENGL_GLFW_TARGET}
    OpenGL::GL
    imgui
    nlohmann_json::nlohmann_json
)

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

install(TARGETS OpenGLApp RUNTIME DESTINATION bin)
install(DIRECTORY "${APP_SOURCE_ROOT}/Assets/" DESTINATION bin/Assets)
