cmake_minimum_required(VERSION 3.19)
project(ICEBERG)

message(STATUS "Building ${PROJECT_NAME}")

add_executable(${PROJECT_NAME}
    src/Iceberg.cpp
    src/ProjectSelection.cpp
    src/Editor.cpp
    src/Hierarchy.cpp
    src/Inspector.cpp
    src/Assets.cpp
    src/Viewport.cpp
    src/AssetsRenderer.cpp
    src/Dialog.cpp
    ${IMGUI_SRC}
    ${IMGUIZMO_SRC}
    ${GL3W_SRC} 
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)

target_link_libraries(${PROJECT_NAME} PUBLIC ICE DearImXML glfw)

file(COPY ${ICE_ROOT_SOURCE_DIR}/Assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/XML DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
