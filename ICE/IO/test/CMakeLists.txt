cmake_minimum_required(VERSION 3.19)
project(io-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(MaterialsTestSuite
        MaterialExporterTest.cpp
        MaterialLoaderTest.cpp
)

add_test(NAME MaterialsTestSuite
        COMMAND MaterialsTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:AssetTestSuite>)

target_link_libraries(MaterialsTestSuite
        PRIVATE
        gtest_main
        io)

add_executable(MeshLoaderTestSuite
    MeshLoaderTest.cpp
)

add_test(NAME MeshLoaderTestSuite
        COMMAND MeshLoaderTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:MeshLoaderTestSuite>)

target_link_libraries(MeshLoaderTestSuite
        PRIVATE
        gtest_main
        io)

add_custom_command(
    TARGET MeshLoaderTestSuite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/cube.obj"
            "${CMAKE_CURRENT_BINARY_DIR}"
)