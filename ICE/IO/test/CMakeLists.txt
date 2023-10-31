cmake_minimum_required(VERSION 3.19)
project(materials-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(MaterialsTestSuite
        MaterialExporterTest.cpp
)

add_test(NAME MaterialsTestSuite
        COMMAND MaterialsTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:AssetTestSuite>)

target_link_libraries(MaterialsTestSuite
        PRIVATE
        gtest_main
        io)


