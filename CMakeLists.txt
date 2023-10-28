cmake_minimum_required(VERSION 3.19)
project(ICE_ROOT)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_CXX_STANDARD 20)

include(cmake/fetch_dependencies.cmake)

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(APPLE)
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(OPENGL_LIBRARY OpenGL REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
elseif(UNIX)
	link_libraries(-lstdc++fs)
endif()
include_directories(includes)
		
set(IMGUI_SRC 
	${CMAKE_SOURCE_DIR}/includes/ImGUI/imgui_impl_glfw.cpp
	${CMAKE_SOURCE_DIR}/includes/ImGUI/imgui_impl_opengl3.cpp
	${CMAKE_SOURCE_DIR}/includes/ImGUI/imgui.cpp
	${CMAKE_SOURCE_DIR}/includes/ImGUI/imgui_widgets.cpp
	${CMAKE_SOURCE_DIR}/includes/ImGUI/imgui_draw.cpp
	${CMAKE_SOURCE_DIR}/includes/ImGUI/imgui_tables.cpp
)

set(GL3W_SRC 
	${CMAKE_SOURCE_DIR}/includes/GL/gl3w.c
)


add_subdirectory(ICE)
add_subdirectory(ICEBERG)
