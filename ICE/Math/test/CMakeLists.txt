cmake_minimum_required(VERSION 3.19)
project(math-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(MathTestSuite
        AABBTest.cpp
)

add_test(NAME MathTestSuite
        COMMAND MathTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:MathTestSuite>)

target_link_libraries(MathTestSuite
        PRIVATE
        gtest_main
        core
        math)


