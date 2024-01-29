cmake_minimum_required(VERSION 3.19)
project(graphics_api_None)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} INTERFACE)

#target_link_libraries(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

enable_testing()
#add_subdirectory(test)
