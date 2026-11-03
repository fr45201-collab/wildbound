# ============================================================================
# WILDBOUND TEST SUITE (CTest)
# ============================================================================

# 1. Core Configuration Test
add_executable(test_config
    core/test_config.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_config PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_features(test_config PRIVATE cxx_std_20)

add_test(NAME CoreConfigTest COMMAND test_config)
set_tests_properties(CoreConfigTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 2. Lua Scripting & Config Parser Test
add_executable(test_lua
    scripting/test_lua.cpp
    ${CMAKE_SOURCE_DIR}/src/scripting/LuaManager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/EngineConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Logger.cpp
)
target_include_directories(test_lua PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${sol2_SOURCE_DIR}/include
    ${lua_SOURCE_DIR}/lua-5.4.7/src
)
target_compile_features(test_lua PRIVATE cxx_std_20)
target_link_libraries(test_lua PRIVATE sol2 lua_static)

add_test(NAME LuaScriptingTest COMMAND test_lua)
set_tests_properties(LuaScriptingTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# 3. Executable Launch / Smoke Test (--test-run)
add_test(NAME ExecutableSmokeTest COMMAND wildbound --test-run)
set_tests_properties(ExecutableSmokeTest PROPERTIES WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

