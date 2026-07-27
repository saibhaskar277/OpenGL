cmake_minimum_required(VERSION 3.16)

project(OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APP_SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL")

# ---------------------------------------------------------------------------
# GLFW: system package if available, otherwise fetch and build from source
# (needed on Linux/macOS; also works on Windows without the vendored .lib)
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
    include(FetchContent)
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
# App sources (mirrors OpenGL.vcxproj)
# ---------------------------------------------------------------------------
set(OPENGL_SOURCES
    "${APP_SOURCE_ROOT}/glad.c"
    "${APP_SOURCE_ROOT}/Sandbox/src/main.cpp"
    "${APP_SOURCE_ROOT}/Engine/Core/Window.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/CubemapShadowMap.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/DirectionalShadowPass.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/FlyCameraController.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/IndexBuffer.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/LightShadowSystem.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Lighting/LightingEnvironment.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Material.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Mesh.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Model.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/PerspectiveCamera.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/PrimitiveFactory.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Renderer.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Shader.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/ShadowMap.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/TangentSpace.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/Texture.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/VertexArray.cpp"
    "${APP_SOURCE_ROOT}/Engine/Renderer/VertexBuffer.cpp"
    "${APP_SOURCE_ROOT}/Engine/Scene/Transform.cpp"
)

add_executable(OpenGLApp ${OPENGL_SOURCES})

target_include_directories(OpenGLApp PRIVATE
    "${APP_SOURCE_ROOT}"
    "${APP_SOURCE_ROOT}/Dependencies/include"
)

target_link_libraries(OpenGLApp PRIVATE
    ${OPENGL_GLFW_TARGET}
    OpenGL::GL
)

if(APPLE)
    target_compile_definitions(OpenGLApp PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Shaders are loaded as "Assets/Shaders/..." relative to the working directory.
add_custom_command(TARGET OpenGLApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${APP_SOURCE_ROOT}/Assets"
        "$<TARGET_FILE_DIR:OpenGLApp>/Assets"
    COMMENT "Copying Assets next to OpenGLApp"
)

# When generating a Visual Studio solution via CMake, run from the exe folder.
set_target_properties(OpenGLApp PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:OpenGLApp>"
)

install(TARGETS OpenGLApp RUNTIME DESTINATION bin)
install(DIRECTORY "${APP_SOURCE_ROOT}/Assets/" DESTINATION bin/Assets)
