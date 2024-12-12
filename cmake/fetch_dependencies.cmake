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
include(FetchContent)
FetchContent_Declare(
  GLFW
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG master
)
FetchContent_MakeAvailable(GLFW)

message(STATUS "Fetching Assimp")
include(FetchContent)
FetchContent_Declare(
  Assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG master
)
FetchContent_MakeAvailable(Assimp)
