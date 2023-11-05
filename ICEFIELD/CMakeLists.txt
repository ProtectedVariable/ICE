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
