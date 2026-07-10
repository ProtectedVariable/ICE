cmake_minimum_required(VERSION 3.19)
project(storage)

message(STATUS "Building ${PROJECT_NAME} module")

# Header-only after removing the empty Filesystem stub (JsonParser is the only content).
add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} INTERFACE
    nlohmann_json
)

target_include_directories(${PROJECT_NAME} INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
#add_subdirectory(test)
