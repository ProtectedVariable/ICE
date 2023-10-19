cmake_minimum_required(VERSION 3.19)
project(graphics_api)

message(STATUS "Building ${PROJECT_NAME} module")

add_subdirectory(None)
add_subdirectory(OpenGL)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    GraphicsAPI.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    graphics
    graphics_api_OpenGL
    graphics_api_None
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

enable_testing()
#add_subdirectory(test)
