cmake_minimum_required(VERSION 3.19)
project(util)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/BufferUtils.cpp
	src/ICEException.cpp
	src/Logger.cpp
	src/OBJLoader.cpp
	src/WindowFactory.cpp
	src/GLFWWindow.cpp
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
	graphics
	glfw
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
#add_subdirectory(test)
