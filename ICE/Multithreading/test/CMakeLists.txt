cmake_minimum_required(VERSION 3.19)
project(multithreading-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

find_package(Threads REQUIRED)

add_executable(JobSchedulerTestSuite
        JobSchedulerTest.cpp
)

add_test(NAME JobSchedulerTestSuite
        COMMAND JobSchedulerTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:JobSchedulerTestSuite>)

target_link_libraries(JobSchedulerTestSuite
        PRIVATE
        gtest_main
        multithreading
        Threads::Threads
)
