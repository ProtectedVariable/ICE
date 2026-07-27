cmake_minimum_required(VERSION 3.19)
project(container-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(HandlePoolTestSuite
        HandlePoolTest.cpp
)

add_test(NAME HandlePoolTestSuite
        COMMAND HandlePoolTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:HandlePoolTestSuite>)

target_link_libraries(HandlePoolTestSuite
        PRIVATE
        gtest_main
        container
)
