cmake_minimum_required(VERSION 3.19)
project(graphics-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(HandlePoolTestSuite
        HandlePoolTest.cpp
)

# HandlePool.h / GpuHandle.h are header-only and GL-free, so pull in just the include directory
# rather than linking the whole graphics library (which would drag in OpenGL/GLFW).
target_include_directories(HandlePoolTestSuite PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_test(NAME HandlePoolTestSuite
        COMMAND HandlePoolTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:HandlePoolTestSuite>)

target_link_libraries(HandlePoolTestSuite
        PRIVATE
        gtest_main
)
