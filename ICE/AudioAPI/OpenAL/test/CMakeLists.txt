cmake_minimum_required(VERSION 3.19)
project(audio-openal-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

# Exercises the streaming buffer-queue machinery against a REAL OpenAL device: priming, underrun
# tolerance, looping by rewind, and releasing a voice while a decode job is in flight. Each test
# skips itself when no device can be opened (the normal state on CI), so the suite is safe to run
# everywhere; the device-free paths are covered by the audio module's null/mock suites.
add_executable(OpenALStreamingTestSuite
        OpenALStreamingTest.cpp
)

add_test(NAME OpenALStreamingTestSuite
        COMMAND OpenALStreamingTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:OpenALStreamingTestSuite>)

target_link_libraries(OpenALStreamingTestSuite
        PRIVATE
        gtest_main
        audio_api_openal
)
