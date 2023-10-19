cmake_minimum_required(VERSION 3.19)
project(graphics)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/Camera.cpp
    src/ForwardRenderer.cpp
    src/Material.cpp
    src/Mesh.cpp
    src/Skybox.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    assets
    math
    graphics_api
    entity
    scene
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src)

enable_testing()
#add_subdirectory(test)
