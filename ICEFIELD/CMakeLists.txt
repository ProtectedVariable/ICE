cmake_minimum_required(VERSION 3.19)
project(ICEFIELD)

message(STATUS "Building ${PROJECT_NAME}")

add_executable(${PROJECT_NAME}
    icefield.cpp
    ${GL3W_SRC} 
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ICE glfw)

# Configure-time copy (first-time setup). NOTE: file(COPY) runs only at CMake configure, so on its
# own it leaves the staged Assets stale when files are added/changed and only the build is re-run --
# which is exactly how a newly added shader (e.g. ui.shader.json) goes missing at runtime.
file(COPY ${ICE_ROOT_SOURCE_DIR}/Assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ImportAssets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Re-sync Assets after every build so added/edited shaders, fonts and meshes always reach the
# executable without a reconfigure. copy_directory refreshes changed files in place.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${ICE_ROOT_SOURCE_DIR}/Assets ${CMAKE_CURRENT_BINARY_DIR}/Assets
    COMMENT "Syncing Assets to the ICEFIELD build directory")
