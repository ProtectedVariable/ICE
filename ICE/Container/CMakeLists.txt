cmake_minimum_required(VERSION 3.19)
project(container)

message(STATUS "Building ${PROJECT_NAME} module")

# Generic, dependency-free containers shared across layers. Header-only and -- critically -- links
# NOTHING, so any module may depend on it without risking a cycle. HandlePool lived in `graphics`
# until the audio layer needed the same generational-handle pool for its voice pool; `util` (the
# obvious home) is not a leaf -- it reaches up to `graphics`/`core` via EngineHelper.h, so moving
# HandlePool there would have forced `graphics -> util` and closed a new cycle. See
# docs/module_dependencies.md.
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
add_subdirectory(test)
