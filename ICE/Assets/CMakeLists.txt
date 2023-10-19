cmake_minimum_required(VERSION 3.19)
project(assets)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
        src/AssetBank.cpp
		src/TextureLoader.cpp
		src/MeshLoader.cpp
        src/ShaderLoader.cpp 
        src/MaterialLoader.cpp
		src/AssetPath.cpp)

target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
    graphics 
    util
    storage
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src)

enable_testing()
#add_subdirectory(test)
