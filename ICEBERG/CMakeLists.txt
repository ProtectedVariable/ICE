cmake_minimum_required(VERSION 3.19)
project(ICEBERG)

message(STATUS "Building ${PROJECT_NAME}")

set(ICE_GUI_SRC
	# GUI
	ICEGUI.cpp
	ICEGUI.h
	HierarchyPane.cpp
	HierarchyPane.h 
	ICEPane.h 
	InspectorPane.cpp 
	InspectorPane.h 
	Components/UIComponentRenderer.cpp
	Components/UIComponentRenderer.h 
	AssetPane.cpp 
	AssetPane.h
	AssetViewPane.cpp 
	AssetViewPane.h 
	AssetContentPane.cpp 
	AssetContentPane.h 
	ProjectSelectorWindow.cpp 
	ProjectSelectorWindow.h
	NewMaterialPane.cpp 
	NewMaterialPane.h 
	SceneParamPane.cpp 
	SceneParamPane.h
)

add_executable(${PROJECT_NAME} ${ICE_GUI_SRC})
target_link_libraries(ICE)