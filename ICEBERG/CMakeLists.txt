cmake_minimum_required(VERSION 3.19)
project(ICEBERG)

message(STATUS "Building ${PROJECT_NAME}")

add_executable(${PROJECT_NAME} Iceberg.cpp "UI/ProjectSelectionWindow.h" "UI/UIElement.h")

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} INTERFACE ICE PUBLIC glfw DearImXML)

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/XML
                ${CMAKE_CURRENT_BINARY_DIR}/XML)