cmake_minimum_required(VERSION 3.19)
project(audio)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/audio_decoders_impl.cpp
    src/AudioDecoder.cpp
    src/AudioClipLoader.cpp
    src/AudioRegistry.cpp
    src/AudioEngine.cpp
    src/FileAudioStream.cpp
)

# Backend-agnostic audio layer. Deliberately does NOT link `graphics`, `scene` or `system`: the
# only scene coupling in the audio stack lives in AudioSystem (the `system` module). See
# docs/module_dependencies.md.
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    assets      # AudioClip is a CPU-side Asset, and this is where AssetUID/AssetPath live
    math
    container       # VoicePool is a HandlePool (generational handles, same as the GPU pools)
    multithreading  # streaming decode runs as detached jobs on the shared JobScheduler
    PRIVATE
    dr_libs     # wav / mp3 / flac decoding -- implementation detail, not in the public headers
    stb_vorbis  # ogg vorbis decoding
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

enable_testing()
add_subdirectory(test)
