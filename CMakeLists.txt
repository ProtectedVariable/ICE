cmake_minimum_required(VERSION 3.17)
project(ICE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

set(CMAKE_CXX_STANDARD 14)
add_library(glfw3 STATIC IMPORTED includes/OBJLoader/OBJLoader.h)
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
        ICE/src/Scene/Scene.cpp ICE/src/Scene/Scene.h ICE/src/Util/OBJLoader.cpp ICE/src/Util/OBJLoader.h ICE/src/Graphics/Renderer.h ICE/src/Graphics/RenderSystem.cpp ICE/src/Graphics/RenderSystem.h ICE/src/Core/System.h ICE/src/Graphics/RendererParams.h ICE/src/Graphics/RenderCommand.h ICE/src/Graphics/Buffers.h ICE/src/Graphics/FrameBuffer.h ICE/src/Graphics/Context.h ICE/src/Graphics/VertexArray.h ICE/src/Graphics/API/GraphicsAPI.h ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.cpp ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.h ICE/src/Graphics/API/OpenGL/OpenGLContext.cpp ICE/src/Graphics/API/OpenGL/OpenGLContext.h ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.cpp ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.h ICE/src/Graphics/API/OpenGL/OpenGLBuffers.cpp ICE/src/Graphics/API/OpenGL/OpenGLBuffers.h ICE/src/Graphics/API/GraphicsAPI.cpp)

add_executable(ICE_TEST
        ICE/src/Scene/Entity.cpp ICE/src/Scene/Entity.h ICE/src/Scene/Component.h ICE/src/Scene/TransformComponent.cpp ICE/src/Scene/TransformComponent.h ICE/src/Scene/RenderComponent.cpp ICE/src/Scene/RenderComponent.h ICE/src/Graphics/Material.cpp ICE/src/Graphics/Material.h ICE/src/Graphics/Mesh.cpp ICE/src/Graphics/Mesh.h
        ICE/test/Tests.cpp
        ICE/src/Scene/Scene.cpp ICE/src/Scene/Scene.h ICE/src/Util/OBJLoader.cpp ICE/src/Util/OBJLoader.h ICE/src/Graphics/Renderer.h ICE/src/Graphics/RenderSystem.cpp ICE/src/Graphics/RenderSystem.h ICE/src/Core/System.h ICE/src/Graphics/RendererParams.h ICE/src/Graphics/Buffers.h ICE/src/Graphics/FrameBuffer.h ICE/src/Graphics/Context.h ICE/src/Graphics/VertexArray.h ICE/src/Graphics/API/GraphicsAPI.h ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.cpp ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.h ICE/src/Graphics/API/OpenGL/OpenGLContext.cpp ICE/src/Graphics/API/OpenGL/OpenGLContext.h ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.cpp ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.h ICE/src/Graphics/API/OpenGL/OpenGLBuffers.cpp ICE/src/Graphics/API/OpenGL/OpenGLBuffers.h ICE/src/Graphics/API/GraphicsAPI.cpp)


target_link_libraries(ICE glfw3 ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY})
