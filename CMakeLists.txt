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

set(ICE_SRC 
		# Rendering API
		includes/GL/gl3w.h
		
		# ImGUI
		includes/ImGUI/gl3w.c
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
		
		# External Libs
		includes/OBJLoader/tiny_obj_loader.h
		includes/stb/stb_image.h 
		includes/json/json.h
		
		# Graphics
		ICE/src/Graphics/Material.cpp ICE/src/Graphics/Material.h ICE/src/Graphics/Mesh.cpp ICE/src/Graphics/Mesh.h
		ICE/src/Graphics/Renderer.h
		ICE/src/Graphics/RendererConfig.h ICE/src/Graphics/RenderCommand.h ICE/src/Graphics/Buffers.h
		ICE/src/Graphics/Framebuffer.h ICE/src/Graphics/Context.h ICE/src/Graphics/VertexArray.h ICE/src/Graphics/API/GraphicsAPI.h
		ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.cpp ICE/src/Graphics/API/OpenGL/OpenGLVertexArray.h 
		ICE/src/Graphics/API/OpenGL/OpenGLContext.cpp ICE/src/Graphics/API/OpenGL/OpenGLContext.h
		ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.cpp ICE/src/Graphics/API/OpenGL/OpenGLRendererAPI.h 
		ICE/src/Graphics/API/OpenGL/OpenGLBuffers.cpp ICE/src/Graphics/API/OpenGL/OpenGLBuffers.h ICE/src/Graphics/API/GraphicsAPI.cpp 
		ICE/src/Graphics/Shader.h ICE/src/Graphics/API/OpenGL/OpenGLShader.cpp ICE/src/Graphics/API/OpenGL/OpenGLShader.h
		ICE/src/Graphics/ForwardRenderer.cpp ICE/src/Graphics/ForwardRenderer.h ICE/src/Graphics/Camera.cpp ICE/src/Graphics/Camera.h
		ICE/src/Graphics/API/OpenGL/OpenGLFramebuffer.cpp ICE/src/Graphics/API/OpenGL/OpenGLFramebuffer.h
		ICE/src/Graphics/Texture.h ICE/src/Graphics/API/OpenGL/OpenGLTexture2D.cpp ICE/src/Graphics/API/OpenGL/OpenGLTexture.h
		ICE/src/Graphics/API/OpenGL/OpenGLTextureCube.cpp ICE/src/Graphics/Skybox.cpp ICE/src/Graphics/Skybox.h
		ICE/src/Graphics/API/None/NoneGraphics.h
		
		# Math
		ICE/src/Math/ICEMath.h ICE/src/Math/ICEMath.cpp 
		ICE/src/Math/AABB.cpp ICE/src/Math/AABB.h
		
		# Scene
		ICE/src/Scene/Scene.cpp ICE/src/Scene/Scene.h 
		
		# Utils
		ICE/src/Util/OBJLoader.cpp ICE/src/Util/OBJLoader.h 
		ICE/src/Util/BufferUtils.cpp ICE/src/Util/BufferUtils.h 
		ICE/src/Util/Logger.cpp ICE/src/Util/Logger.h
		ICE/src/Util/ICEException.cpp ICE/src/Util/ICEException.h
		
		# Core
		ICE/src/Core/ICEEngine.cpp ICE/src/Core/ICEEngine.h
		
		# Assets
		ICE/src/Assets/AssetBank.cpp ICE/src/Assets/AssetBank.h
		ICE/src/Assets/Asset.h ICE/src/Assets/ResourceLoader.h ICE/src/Assets/TextureLoader.cpp ICE/src/Assets/TextureLoader.h ICE/src/Assets/Resource.h 
		ICE/src/Assets/IResourceLoader.h
		ICE/src/Assets/MeshLoader.cpp ICE/src/Assets/MeshLoader.h ICE/src/Assets/ShaderLoader.cpp ICE/src/Assets/ShaderLoader.h ICE/src/Assets/MaterialLoader.cpp 
		ICE/src/Assets/MaterialLoader.h ICE/src/Assets/AssetPath.cpp ICE/src/Assets/AssetPath.h
		
		# Platform
		ICE/src/Platform/FileUtils.cpp ICE/src/Platform/FileUtils.h 
		
		# IO
		ICE/src/IO/Project.cpp ICE/src/IO/Project.h 
		ICE/src/IO/EngineConfig.cpp ICE/src/IO/EngineConfig.h
		
		# Filesystem
		ICE/src/Filesystem/Filesystem.cpp ICE/src/Filesystem/Filesystem.h
		ICE/src/Filesystem/JsonParser.h
		
		# ECS
		ICE/src/ECS/System.h
		ICE/src/ECS/RenderSystem.h ICE/src/ECS/RenderSystem.cpp
		ICE/src/ECS/Registry.h ICE/src/ECS/Entity.h ICE/src/ECS/Entity.h ICE/src/ECS/Component.h ICE/src/ECS/TransformComponent.h ICE/src/ECS/RenderComponent.h
		ICE/src/ECS/CameraComponent.h ICE/src/ECS/LightComponent.h)

set(ICE_GUI_SRC
	# GUI
	ICE/src/GUI/ICEGUI.cpp ICE/src/GUI/ICEGUI.h
	ICE/src/GUI/HierarchyPane.cpp
	ICE/src/GUI/HierarchyPane.h ICE/src/GUI/ICEPane.h ICE/src/GUI/InspectorPane.cpp ICE/src/GUI/InspectorPane.h ICE/src/GUI/Components/UIComponentRenderer.cpp
	ICE/src/GUI/Components/UIComponentRenderer.h ICE/src/GUI/AssetPane.cpp ICE/src/GUI/AssetPane.h
	ICE/src/GUI/AssetViewPane.cpp ICE/src/GUI/AssetViewPane.h ICE/src/GUI/AssetContentPane.cpp ICE/src/GUI/AssetContentPane.h 
	ICE/src/GUI/ProjectSelectorWindow.cpp ICE/src/GUI/ProjectSelectorWindow.h
	ICE/src/GUI/NewMaterialPane.cpp ICE/src/GUI/NewMaterialPane.h 
	ICE/src/GUI/SceneParamPane.cpp ICE/src/GUI/SceneParamPane.h
)
		
add_library(ICE SHARED "${EXTRA_SRC}" ${ICE_SRC})
add_executable(ICEBERG ${ICE_GUI_SRC})
target_link_libraries(ICEBERG ICE)

file(COPY ICE/Assets DESTINATION ${CMAKE_BINARY_DIR})


if(APPLE)
	enable_testing()
	find_package(GTest REQUIRED)
	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	foreach (_variableName ${_variableNames})
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach()
	include_directories(${GTEST_INCLUDE_DIRS})
	add_executable(ICE_TEST
			"${EXTRA_SRC}"
			ICE/test/Tests.cpp
			${ICE_SRC} ICE/test/AssetBankTest.h)
	target_compile_definitions(ICE_TEST PUBLIC ICE_TEST)
	target_link_libraries(ICE glfw ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY})
	target_link_libraries(ICE_TEST glfw ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} ${GTEST_LIBRARIES})
elseif(WIN32)
	target_link_libraries(ICE glfw)
else()
	enable_testing()
	find_package(GTest REQUIRED)
	include_directories(${GTEST_INCLUDE_DIRS})
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

	add_executable(ICE_TEST
			"${EXTRA_SRC}"
			ICE/test/Tests.cpp
			${ICE_SRC} 
			ICE/test/AssetBankTest.h
			ICE/test/SceneTest.h)
	target_compile_definitions(ICE_TEST PUBLIC ICE_TEST)

	include_directories(${GTK3_INCLUDE_DIRS})
	link_directories(${GTK3_LIBRARY_DIRS})

	add_definitions(${GTK3_CFLAGS_OTHER})

	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIRS})

	target_link_libraries(ICE glfw ${GTK3_LIBRARIES} ${OPENGL_LIBRARIES} stdc++fs)
	target_link_libraries(ICE_TEST glfw ${GTK3_LIBRARIES} ${OPENGL_LIBRARIES}  ${GTEST_LIBRARIES} stdc++fs)
endif()