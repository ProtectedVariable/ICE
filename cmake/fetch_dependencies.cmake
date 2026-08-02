include(FetchContent)

message(STATUS "Fetching googletest")
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.13.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(googletest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

message(STATUS "Fetching GLFW")
FetchContent_Declare(
  GLFW
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(GLFW)

message(STATUS "Fetching Assimp")
FetchContent_Declare(
  Assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG v6.0.5
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(Assimp)

message(STATUS "Fetching DearImXML")
# Don't build DearImXML's example app: it links imgui_impl_glfw which needs X11
# libs that its example target doesn't request, breaking the Linux build.
set(BUILD_IMXML_EXAMPLE OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  DearImXML
  GIT_REPOSITORY https://github.com/ProtectedVariable/DearImXML.git
  GIT_TAG development
)
FetchContent_MakeAvailable(DearImXML)

message(STATUS "Fetching nlohmann_json...")
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.12.0
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(json)


# --- Audio -------------------------------------------------------------------------------------
# OpenAL Soft is the audio backend (3D spatialization, HRTF, EFX reverb/filters). Built STATIC to
# preserve this project's "every dependency links static" property: ICE is itself LGPL-2.1, the
# same license as OpenAL Soft, so static linking imposes no obligation ICE does not already carry.
message(STATUS "Fetching OpenAL Soft")
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_CONFIG OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_HRTF_DATA OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_UTILS OFF CACHE BOOL "" FORCE)
set(LIBTYPE "STATIC" CACHE STRING "" FORCE)
FetchContent_Declare(
        openal
        GIT_REPOSITORY https://github.com/kcat/openal-soft.git
        GIT_TAG 1.25.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(openal)

# Audio decoders. OpenAL is playback-only -- it takes raw PCM and nothing else -- so decoding is
# ours. All four are single-header and public domain, matching the vendored stb_image precedent.
#
# NOTE: neither upstream publishes release tags, so these are pinned to specific commits rather
# than a branch. Bump deliberately; do NOT switch these to `master` (non-reproducible builds).
message(STATUS "Fetching dr_libs (wav/mp3/flac decoders)")
FetchContent_Declare(
        dr_libs
        GIT_REPOSITORY https://github.com/mackron/dr_libs.git
        GIT_TAG 34a89ffe6bfc4d78db6888fef76cd408dba18185
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(dr_libs)

add_library(dr_libs INTERFACE)
target_include_directories(dr_libs INTERFACE ${dr_libs_SOURCE_DIR})

message(STATUS "Fetching stb (stb_vorbis)")
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG 31c1ad37456438565541f4919958214b6e762fb4
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(stb)

add_library(stb_vorbis INTERFACE)
target_include_directories(stb_vorbis INTERFACE ${stb_SOURCE_DIR})

add_compile_definitions(FT_CONFIG_OPTION_ERROR_STRINGS)
message(STATUS "Fetching FreeType")
FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG VER-2-13-3
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE)
FetchContent_MakeAvailable(freetype)