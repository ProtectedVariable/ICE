cmake_minimum_required(VERSION 3.19)
project(system)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/RenderSystem.cpp
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    graphics
    entity
    components
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
#add_subdirectory(test)
