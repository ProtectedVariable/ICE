cmake_minimum_required(VERSION 3.19)
project(audio_system)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/AudioSystem.cpp
)

# The ECS half of the audio stack: it is the ONLY place where the audio layer meets the scene.
#
# Why this is its own module rather than living in `system` beside RenderSystem: `system` sits
# inside the pre-existing assets <-> graphics <-> scene <-> system cycle (see
# docs/module_dependencies.md). Adding `system -> audio` pulls `audio` into that SCC and the
# module_cycle_check test rejects it -- verified, it reports `audio->assets` and `system->audio` as
# new back-edges. Sitting above both instead keeps `audio` free of any renderer dependency and adds
# no cycle. Fold this into `system` once the assets/graphics debt is paid down.
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    audio
    system
    components
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
add_subdirectory(test)
