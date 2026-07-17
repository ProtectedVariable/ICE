cmake_minimum_required (VERSION 3.8)
project(UI)

add_library(${PROJECT_NAME} STATIC
    src/UI.cpp
    src/UIManager.cpp)
target_link_libraries(${PROJECT_NAME}
    graphics
    graphics_api
    freetype
    nlohmann_json)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${ICE_ROOT_SOURCE_DIR}/includes>
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include)
add_definitions(-DNOMINMAX)

enable_testing()
add_subdirectory(test)
