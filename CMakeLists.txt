cmake_minimum_required(VERSION 3.19)
project(ICE_ROOT)

set(CMAKE_CXX_STANDARD 20)
set(GLFW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/")

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(NOT EXISTS "${GLFW_LIB_PATH}")
	execute_process(COMMAND git clone https://github.com/glfw/glfw.git "${GLFW_LIB_PATH}")
endif()
add_subdirectory(libs/glfw/)

if(APPLE)
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(OPENGL_LIBRARY OpenGL REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
elseif(UNIX)
	link_libraries(-lstdc++fs)
endif()
include_directories(includes)
		
file(COPY ICE/Assets DESTINATION ${CMAKE_BINARY_DIR})

add_subdirectory(ICE)
#add_subdirectory(ICEBERG)
