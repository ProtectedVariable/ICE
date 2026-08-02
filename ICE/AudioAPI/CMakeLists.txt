cmake_minimum_required(VERSION 3.19)
project(audio_api)

message(STATUS "Building ${PROJECT_NAME} backends")

# Container directory only -- no meta-target. See ICE/AudioAPI/OpenAL/CMakeLists.txt for why the
# graphics_api meta-target pattern is not repeated here.
add_subdirectory(OpenAL)
