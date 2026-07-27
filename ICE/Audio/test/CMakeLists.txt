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

# Voice pool, stealing policy, gain/mute, buffer eviction and the silent-backend fallback. Runs
# entirely on MockAudioBackend/NullAudioBackend, so it needs no audio device -- which is what makes
# it meaningful on CI.
add_executable(AudioEngineTestSuite
        AudioEngineTest.cpp
)

add_test(NAME AudioEngineTestSuite
        COMMAND AudioEngineTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:AudioEngineTestSuite>)

target_link_libraries(AudioEngineTestSuite
        PRIVATE
        gtest_main
        audio
)
