cmake_minimum_required(VERSION 3.19)
project(audio-system-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

add_executable(AudioSystemTestSuite
        AudioSystemTest.cpp
)

add_test(NAME AudioSystemTestSuite
        COMMAND AudioSystemTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:AudioSystemTestSuite>)

target_link_libraries(AudioSystemTestSuite
        PRIVATE
        gtest_main
        audio_system
)

# Reuses MockAudioBackend from the audio module's suite rather than duplicating it.
target_include_directories(AudioSystemTestSuite PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../Audio/test)
