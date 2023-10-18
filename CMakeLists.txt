cmake_minimum_required(VERSION 3.25)
project(ICE)

set(CMAKE_CXX_STANDARD 20)
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
		
		# Assets
		
		# Platform
		ICE/src/Platform/FileUtils.cpp ICE/src/Platform/FileUtils.h 
		
		# IO
		ICE/src/IO/Project.cpp ICE/src/IO/Project.h 
		ICE/src/IO/EngineConfig.cpp ICE/src/IO/EngineConfig.h
		
		# Filesystem
		ICE/src/Filesystem/Filesystem.cpp ICE/src/Filesystem/Filesystem.h
		ICE/src/Filesystem/JsonParser.h
		
		# ECS
		"ICE/System/include/System.h"
		"ICE/System/include/RenderSystem.h" ICE/src/ECS/RenderSystem.cpp
		"ICE/System/include/Registry.h" ICE/src/ECS/Entity.h ICE/src/ECS/Entity.h "ICE/Components/include/Component.h" "ICE/Components/include/TransformComponent.h" "ICE/Components/include/RenderComponent.h"
		"ICE/Components/include/CameraComponent.h" "ICE/Components/include/LightComponent.h")

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
		
add_library(ICE STATIC "${EXTRA_SRC}" ${ICE_SRC})
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