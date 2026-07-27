cmake_minimum_required(VERSION 3.19)
project(audio-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(AudioDecoderTestSuite
        AudioDecoderTest.cpp
)

add_test(NAME AudioDecoderTestSuite
        COMMAND AudioDecoderTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:AudioDecoderTestSuite>)

target_link_libraries(AudioDecoderTestSuite
        PRIVATE
        gtest_main
        audio
)
