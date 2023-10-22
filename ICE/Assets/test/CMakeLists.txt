cmake_minimum_required(VERSION 3.19)
project(assets-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_compile_definitions(TINYOBJLOADER_IMPLEMENTATION)

add_executable(AssetBankTest
        AssetBankTest.cpp)

add_test(NAME AssetBankTest
        COMMAND AssetBankTest
        WORKING_DIRECTORY $<TARGET_FILE_DIR:AssetBankTest>)

target_link_libraries(AssetBankTest
        PRIVATE
        gtest_main
        graphics_api
        assets)


