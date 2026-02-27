cmake_minimum_required(VERSION 3.19)
project(utils-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(UtilsTestSuite
        FrustumCullingTests.cpp
        HelpersTest.cpp
)

add_test(NAME UtilsTestSuite
        COMMAND UtilsTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:UtilsTestSuite>)

target_link_libraries(UtilsTestSuite
        PRIVATE
        gtest_main
        core
        util)


