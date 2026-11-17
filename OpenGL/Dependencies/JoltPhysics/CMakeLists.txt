cmake_minimum_required(VERSION 3.16)

project(JoltPhysics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PHYSICS_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/Jolt")

# Include the Jolt.cmake file which defines the Jolt target
include("${CMAKE_CURRENT_SOURCE_DIR}/Jolt/Jolt.cmake")

# The Jolt target is already created by Jolt.cmake
# Just need to ensure it's available for the parent scope
set_target_properties(Jolt PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)