cmake_minimum_required(VERSION 3.20)
project(os_mockup LANGUAGES C CXX)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# -----------------------------------------------------------------------------
# Dependency Management via FetchContent
# -----------------------------------------------------------------------------
include(FetchContent)

# 1. GLFW
message(STATUS "Fetching GLFW...")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

# 2. Dear ImGui
message(STATUS "Fetching Dear ImGui...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.8
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw imgui)

# 3. Find System OpenGL
find_package(OpenGL REQUIRED)

# -----------------------------------------------------------------------------
# GLAD Static Library (Bundled OpenGL 3.3 Core Loader)
# -----------------------------------------------------------------------------
add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

# -----------------------------------------------------------------------------
# Dear ImGui Static Library Target
# -----------------------------------------------------------------------------
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(imgui PUBLIC
    glfw
    glad
    OpenGL::GL
)

# -----------------------------------------------------------------------------
# Main OS Mockup Emulator Target
# -----------------------------------------------------------------------------
set(OS_MOCKUP_SOURCES
    src/Main.cpp
    src/Core/Types.h
    src/Core/SystemContext.h
    src/Core/SystemContext.cpp
    src/Boot/BootScreens.h
    src/Boot/BootScreens.cpp
    src/Desktop/DesktopBackground.h
    src/Desktop/DesktopBackground.cpp
    src/Desktop/Taskbar.h
    src/Desktop/Taskbar.cpp
    src/Apps/TaskManagerWindow.h
    src/Apps/TaskManagerWindow.cpp
    src/Apps/FileExplorerWindow.h
    src/Apps/FileExplorerWindow.cpp
    src/Apps/TerminalWindow.h
    src/Apps/TerminalWindow.cpp
    src/Apps/SystemInfoWindow.h
    src/Apps/SystemInfoWindow.cpp
    src/Apps/NotepadWindow.h
    src/Apps/NotepadWindow.cpp
    src/Compositor/WindowCompositor.h
    src/Compositor/WindowCompositor.cpp
)

add_executable(os_mockup ${OS_MOCKUP_SOURCES})

target_include_directories(os_mockup PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

target_link_libraries(os_mockup PRIVATE
    imgui
    glad
    glfw
    OpenGL::GL
)

# Platform-specific configurations
if (MSVC)
    target_compile_options(os_mockup PRIVATE /W4 /permissive-)
else()
    target_compile_options(os_mockup PRIVATE -Wall -Wextra -Wpedantic)
endif()

message(STATUS "Configured os_mockup executable target successfully.")
