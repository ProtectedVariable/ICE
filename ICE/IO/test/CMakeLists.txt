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

add_executable(ModelLoaderTestSuite
    ModelLoaderTest.cpp
)

add_test(NAME ModelLoaderTestSuite
        COMMAND ModelLoaderTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:ModelLoaderTestSuite>)

target_link_libraries(ModelLoaderTestSuite
        PRIVATE
        gtest_main
        io)

add_custom_command(
    TARGET ModelLoaderTestSuite POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/cube.obj"
            $<TARGET_FILE_DIR:ModelLoaderTestSuite>
)

add_executable(ProjectTestSuite
    ProjectTest.cpp
)

add_test(NAME ProjectTestSuite
        COMMAND ProjectTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:ProjectTestSuite>)

target_link_libraries(ProjectTestSuite
        PRIVATE
        gtest_main
        io
        # Project constructs an OpenGLFactory (GL-free at construction); its vtable pulls the concrete
        # OpenGL symbols, so link the OpenGL backend.
        graphics_api_OpenGL)

