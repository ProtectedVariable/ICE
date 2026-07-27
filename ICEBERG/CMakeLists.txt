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
    src/MaterialEditor.cpp
    src/ShaderEditor.cpp
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

# Configure-time copy (first-time setup). NOTE: file(COPY) runs only at CMake configure, so on its
# own it leaves these staged copies stale when files are added or edited and only the build is
# re-run -- which is exactly how a newly added widget XML goes missing at runtime. Same hazard the
# ICEFIELD Assets copy documents.
file(COPY ${ICE_ROOT_SOURCE_DIR}/Assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/XML DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/EditorAssets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Re-sync after every build so added/edited XML layouts, shaders and editor assets always reach the
# executable without a reconfigure. copy_directory refreshes changed files in place.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/XML ${CMAKE_CURRENT_BINARY_DIR}/XML
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ICE_ROOT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/EditorAssets ${CMAKE_CURRENT_BINARY_DIR}/EditorAssets
    COMMENT "Syncing editor XML/Assets to the ICEBERG build directory")
