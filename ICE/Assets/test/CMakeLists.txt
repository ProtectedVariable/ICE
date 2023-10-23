cmake_minimum_required(VERSION 3.19)
project(assets-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_compile_definitions(TINYOBJLOADER_IMPLEMENTATION)

add_executable(AssetTestSuite
        AssetBankTest.cpp
        AssetPathTest.cpp
)

add_test(NAME AssetTestSuite
        COMMAND AssetTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:AssetTestSuite>)

target_link_libraries(AssetTestSuite
        PRIVATE
        gtest_main
        graphics_api
        assets)


