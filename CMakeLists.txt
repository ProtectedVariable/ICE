cmake_minimum_required(VERSION 3.17)
project(ICE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

set(CMAKE_CXX_STANDARD 14)
add_library(glfw3 STATIC IMPORTED)
set_property(TARGET glfw3 PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/libglfw3.a)
find_library(COCOA_LIBRARY Cocoa REQUIRED)
find_library(OPENGL_LIBRARY OpenGL REQUIRED)
find_library(IOKIT_LIBRARY IOKit REQUIRED)
include_directories(includes)
include_directories(ICE/src)
add_executable(ICE
        ICE/src/main.cpp
        includes/ImGUI/imgui_impl_glfw.cpp
        includes/ImGUI/imgui_impl_opengl3.cpp
        includes/ImGUI/imgui_impl_glfw.h
        includes/ImGUI/imgui_impl_opengl3.h
        includes/ImGUI/gl3w.c
        includes/ImGUI/imgui.cpp
        includes/ImGUI/imgui_widgets.cpp
        includes/ImGUI/imgui_draw.cpp
        includes/ImGUI/imgui_demo.cpp
        ICE/src/Scene/Entity.cpp ICE/src/Scene/Entity.h ICE/src/Scene/Component.h ICE/src/Scene/TransformComponent.cpp ICE/src/Scene/TransformComponent.h ICE/src/Scene/RenderComponent.cpp ICE/src/Scene/RenderComponent.h ICE/src/Graphics/Material.cpp ICE/src/Graphics/Material.h ICE/src/Graphics/Mesh.cpp ICE/src/Graphics/Mesh.h
        )

add_executable(ICE_TEST
        ICE/src/Scene/Entity.cpp ICE/src/Scene/Entity.h ICE/src/Scene/Component.h ICE/src/Scene/TransformComponent.cpp ICE/src/Scene/TransformComponent.h ICE/src/Scene/RenderComponent.cpp ICE/src/Scene/RenderComponent.h ICE/src/Graphics/Material.cpp ICE/src/Graphics/Material.h ICE/src/Graphics/Mesh.cpp ICE/src/Graphics/Mesh.h
        ICE/test/Tests.cpp
        )


target_link_libraries(ICE glfw3 ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY})
