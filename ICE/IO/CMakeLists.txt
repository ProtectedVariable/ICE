cmake_minimum_required(VERSION 3.19)
project(io)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/EngineConfig.cpp
    src/Project.cpp
    src/MaterialExporter.cpp
    src/TextureLoader.cpp
	src/ModelLoader.cpp
    src/ShaderLoader.cpp
    src/MaterialLoader.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    scene
    assets
    graphics_api
    assimp
    platform
    nlohmann_json
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
add_subdirectory(test)
