cmake_minimum_required(VERSION 3.19)
project(audio_api_openal)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/OpenALDevice.cpp
    src/OpenALBackend.cpp
)

# Depends on the audio interface layer in one direction only. Note there is deliberately no
# `audio_api` meta-target mirroring `graphics_api`: that pairing is a tracked baseline cycle
# (graphics_api <-> graphics_api_OpenGL, see docs/module_dependencies.md), and there is no reason
# to reproduce it here. Backends link `audio`; consumers link the backend they want.
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    audio
    PRIVATE
    OpenAL::OpenAL
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
add_subdirectory(test)
