cmake_minimum_required(VERSION 3.19)
project(system-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(SystemTestSuite
        ECSTest.cpp
)

add_test(NAME SystemTestSuite
        COMMAND SystemTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:SystemTestSuite>)

target_link_libraries(SystemTestSuite
        PRIVATE
        gtest_main
        system
        components
)
