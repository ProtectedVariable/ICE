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
