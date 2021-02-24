cmake_minimum_required(VERSION 3.17)
project(ICE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(GLFW_LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/glfw/")

set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)


set(EXTRA_SRC "")
if(NOT EXISTS "${GLFW_LIB_PATH}")
	execute_process(COMMAND git clone https://github.com/glfw/glfw.git "${GLFW_LIB_PATH}")
endif()
add_subdirectory(libs/glfw/)


if(APPLE)
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(OPENGL_LIBRARY OpenGL REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	set(EXTRA_SRC ICE/src/Platform/OSX/dialog.mm)
elseif(WIN32)
	set(EXTRA_SRC ICE/src/Platform/Win32/dialog.cpp)
else()
	link_libraries(-lstdc++fs)
	set(EXTRA_SRC ICE/src/Platform/Linux/dialog.cpp)
endif()
include_directories(includes)
include_directories(ICE/src)

set(ICE_SRC includes/GL/gl3w.h
		includes/ImGUI/gl3w.c
		includes/OBJLoader/tiny_obj_loader.h
		includes/ImGUI/imgui_impl_glfw.cpp
		includes/ImGUI/imgui_impl_opengl3.cpp
		includes/ImGUI/imgui_impl_glfw.h
		includes/ImGUI/imgui_impl_opengl3.h
		includes/ImGUI/imgui.cpp
		includes/ImGUI/imgui_widgets.cpp
		includes/ImGUI/imgui_draw.cpp
		includes/ImGUI/imgui_demo.cpp
		includes/ImGUI/ImCurveEdit.cpp
		includes/ImGUI/ImGradient.cpp
		includes/ImGUI/ImGuizmo.cpp
		includes/ImGUI/ImSequencer.cpp
		ICE/src/Scene/Entity.cpp ICE/src/Scene/Entity.h ICE/src/Scene/Component.h ICE/src/Scene/TransformComponent.cpp ICE/src/Scene/TransformComponent.h ICE/src/Scene/RenderComponent.cpp ICE/src/Scene/RenderComponent.h ICE/src/Graphics/Material.cpp ICE/src/Graphics/Material.h ICE/src/Graphics/Mesh.cpp ICE/src/Graphics/Mesh.h
		ICE/src/Scene/Scene.cpp ICE/src/Scene/Scene.h ICE/src/Util/OBJLoader.cpp ICE/src/Util/OBJLoader.h ICE/src/Graphics/Renderer.h ICE/src/Graphics/RenderSystem.cpp ICE/src/Graphics/RenderSystem.h ICE/src/Core/System.h ICE/src/Graphics/RendererConfig.h ICE/src/Graphics/RenderCommand.h ICE/src/Graphics/Buffers.h
		ICE/src/Graphics/Framebuffer.h ICE/src/Graphics/Context.h ICE/src/Graphics/VertexArray.h ICE/src/Graphics/API/GraphicsAPI.h ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.cpp ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.h ICE/src/Graphics/API/OpenGL/NoneContext.cpp ICE/src/Graphics/API/OpenGL/OpenGLContext.h
		ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.cpp ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.h ICE/src/Graphics/API/OpenGL/NoneGraphics.cpp ICE/src/Graphics/API/OpenGL/OpenGLBuffers.h ICE/src/Graphics/API/GraphicsAPI.cpp ICE/src/Graphics/Shader.h ICE/src/Graphics/API/OpenGL/OpenGLShader.cpp ICE/src/Graphics/API/OpenGL/OpenGLShader.h
		ICE/src/Graphics/ForwardRenderer.cpp ICE/src/Graphics/ForwardRenderer.h ICE/src/Graphics/Camera.cpp ICE/src/Graphics/Camera.h ICE/src/Scene/LightComponent.cpp ICE/src/Scene/LightComponent.h ICE/src/Util/BufferUtils.cpp ICE/src/Util/BufferUtils.h ICE/src/Math/ICEMath.h ICE/src/Util/Logger.cpp ICE/src/Util/Logger.h
		ICE/src/Math/ICEMath.cpp ICE/src/Scene/CameraComponent.cpp ICE/src/Scene/CameraComponent.h ICE/src/Core/ICEEngine.cpp ICE/src/Core/ICEEngine.h ICE/src/GUI/ICEGUI.cpp ICE/src/GUI/ICEGUI.h ICE/src/Graphics/API/OpenGL/OpenGLFramebuffer.cpp ICE/src/Graphics/API/OpenGL/OpenGLFramebuffer.h ICE/src/GUI/HierarchyPane.cpp
		ICE/src/GUI/HierarchyPane.h ICE/src/GUI/ICEPane.h ICE/src/Assets/AssetBank.cpp ICE/src/Assets/AssetBank.h ICE/src/GUI/InspectorPane.cpp ICE/src/GUI/InspectorPane.h ICE/src/GUI/Components/UIComponentRenderer.cpp ICE/src/GUI/Components/UIComponentRenderer.h ICE/src/GUI/AssetPane.cpp ICE/src/GUI/AssetPane.h
		ICE/src/GUI/AssetViewPane.cpp ICE/src/GUI/AssetViewPane.h ICE/src/GUI/AssetContentPane.cpp ICE/src/GUI/AssetContentPane.h ICE/src/Platform/FileUtils.cpp ICE/src/Platform/FileUtils.h ICE/src/IO/Project.cpp ICE/src/IO/Project.h ICE/src/GUI/ProjectSelectorWindow.cpp ICE/src/GUI/ProjectSelectorWindow.h ICE/src/IO/EngineConfig.cpp
		ICE/src/IO/EngineConfig.h ICE/src/GUI/NewMaterialPane.cpp ICE/src/GUI/NewMaterialPane.h ICE/src/Util/ICEException.cpp ICE/src/Util/ICEException.h ICE/src/Graphics/Texture.h ICE/src/Graphics/API/OpenGL/OpenGLTexture2D.cpp ICE/src/Graphics/API/OpenGL/OpenGLTexture.h includes/stb/stb_image.h includes/json/json.h
		ICE/src/Math/AABB.cpp ICE/src/Math/AABB.h ICE/src/Graphics/API/OpenGL/OpenGLTextureCube.cpp ICE/src/Graphics/Skybox.cpp ICE/src/Graphics/Skybox.h ICE/src/GUI/SceneParamPane.cpp ICE/src/GUI/SceneParamPane.h
		ICE/src/Graphics/API/None/NoneGraphics.h)

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(ICE
		"${EXTRA_SRC}"
		${ICE_SRC})

add_executable(ICE_TEST
		"${EXTRA_SRC}"
		ICE/test/Tests.cpp
		${ICE_SRC})

target_compile_definitions(ICE_TEST PUBLIC ICE_TEST)
file(COPY ICE/Assets DESTINATION ${CMAKE_BINARY_DIR})


if(APPLE)
	target_link_libraries(ICE glfw ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY})
	target_link_libraries(ICE_TEST glfw ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} ${GTEST_LIBRARIES})
elseif(WIN32)
	target_link_libraries(ICE glfw)
	target_link_libraries(ICE_TEST glfw)
else()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

	include_directories(${GTK3_INCLUDE_DIRS})
	link_directories(${GTK3_LIBRARY_DIRS})

	add_definitions(${GTK3_CFLAGS_OTHER})

	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIRS})

	target_link_libraries(ICE glfw ${GTK3_LIBRARIES} ${OPENGL_LIBRARIES} stdc++fs)
	target_link_libraries(ICE_TEST glfw ${GTK3_LIBRARIES} ${OPENGL_LIBRARIES}  ${GTEST_LIBRARIES} stdc++fs)
endif()