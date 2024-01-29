cmake_minimum_required(VERSION 3.19)
project(scene-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(SceneTestSuite
        SceneTest.cpp
        SceneGraphTest.cpp
)

add_test(NAME SceneTestSuite
        COMMAND SceneTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:SceneTestSuite>)

target_link_libraries(SceneTestSuite
        PRIVATE
        gtest_main
        scene
)

