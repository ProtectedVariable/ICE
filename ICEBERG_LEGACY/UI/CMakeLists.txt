cmake_minimum_required(VERSION 3.19)
project(ICEBERG_UI)

message(STATUS "Building ${PROJECT_NAME}")

set(ICE_GUI_SRC
	src/ICEGUI.cpp
	src/HierarchyPane.cpp
	src/InspectorPane.cpp 
	Components/UIComponentRenderer.cpp
	Components/UIComponentRenderer.h 
	src/AssetPane.cpp
	src/AssetViewPane.cpp
	src/AssetContentPane.cpp
	src/ProjectSelectorWindow.cpp 
	src/NewMaterialPane.cpp
	src/SceneParamPane.cpp
)

add_library(${PROJECT_NAME} STATIC ${ICE_GUI_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ICE)