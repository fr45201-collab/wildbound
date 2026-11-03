cmake_minimum_required(VERSION 3.20)

# ============================================================================
# WILDBOUND — 2D Fantasy Survival Adventure Engine
# Phase 0: Project Foundation & Architecture
# ============================================================================

project(Wildbound
    VERSION 0.1.0
    DESCRIPTION "2D top-down fantasy survival adventure game engine"
    LANGUAGES CXX C
)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to build automated tests
option(WILDBOUND_BUILD_TESTS "Build Wildbound automated test suite" ON)

# Configure MSVC compiler options
if(MSVC)
    add_compile_options(
        /W4          # High warning level
        /EHsc        # Enable C++ standard exception handling
        /utf-8       # Set source and execution character sets to UTF-8
        /permissive- # Enforce standards conformance
        /Zc:__cplusplus # Correct __cplusplus macro
    )
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# DEPENDENCY MANAGEMENT (CMake FetchContent)
# Pinned versions and immutable release tags
# ============================================================================
include(FetchContent)

# 1. raylib (Graphics Framework) — Tag: 5.5
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)

# 2. Lua 5.4 (Scripting Language) — Tag: v5.4.7
set(LUA_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
set(LUA_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(LUA_BUILD_BINARY OFF CACHE BOOL "" FORCE)
set(LUA_BUILD_COMPILER OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/walterschell/Lua.git
    GIT_TAG        v5.4.7
)

# 3. sol2 (Lua-C++ Binding) — Tag: v3.3.1
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.3.1
)

# 4. Dear ImGui (Developer Interface) — Tag: v1.93.0
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.93.0
)

# 5. rlImGui (raylib Dear ImGui backend) — Tag: Raylib_5_5
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        Raylib_5_5
)

# Populate dependencies
FetchContent_MakeAvailable(raylib lua imgui rlimgui)

# Setup sol2 interface target
FetchContent_GetProperties(sol2)
if(NOT sol2_POPULATED)
    FetchContent_Populate(sol2)
    add_library(sol2 INTERFACE)
    add_library(sol2::sol2 ALIAS sol2)
    target_include_directories(sol2 INTERFACE ${sol2_SOURCE_DIR}/include)
    target_link_libraries(sol2 INTERFACE lua_static)
endif()

# Setup Dear ImGui static library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# Setup rlImGui static library
add_library(rlimgui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PUBLIC raylib imgui)

# ============================================================================
# WILDBOUND ENGINE CORE SOURCES & EXECUTABLE
# ============================================================================

set(WILDBOUND_SOURCES
    src/core/Game.cpp
    src/core/Game.hpp
    src/core/EngineConfig.cpp
    src/core/EngineConfig.hpp
    src/core/Logger.cpp
    src/core/Logger.hpp
    src/graphics/RenderManager.cpp
    src/graphics/RenderManager.hpp
    src/scripting/LuaManager.cpp
    src/scripting/LuaManager.hpp
    src/ui/DevUI.cpp
    src/ui/DevUI.hpp
)

add_executable(wildbound
    src/main.cpp
    ${WILDBOUND_SOURCES}
)

target_include_directories(wildbound PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)

target_link_libraries(wildbound PRIVATE
    raylib
    sol2
    lua_static
    imgui
    rlimgui
)

# Copy engine configuration and scripts to build output folder for seamless execution
add_custom_command(TARGET wildbound POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts
        $<TARGET_FILE_DIR:wildbound>/scripts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:wildbound>/config
    COMMENT "Deploying runtime scripts and config to target output directory"
)

# ============================================================================
# AUTOMATED TESTING (CTest)
# ============================================================================
if(WILDBOUND_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

