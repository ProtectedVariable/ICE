cmake_minimum_required(VERSION 3.19)
project(ui-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

# Exercises the UI layout + pointer hit-testing/event dispatch (dispatchPointer), which is pure
# geometry -- no GL context needed. Links UI for the element headers.
add_executable(UITestSuite
        UIInteractionTest.cpp
)

add_test(NAME UITestSuite
        COMMAND UITestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:UITestSuite>)

target_link_libraries(UITestSuite
        PRIVATE
        gtest_main
        UI
)
