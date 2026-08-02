cmake_minimum_required(VERSION 3.19)
project(multithreading)

message(STATUS "Building ${PROJECT_NAME} module")

add_library(${PROJECT_NAME} INTERFACE)

# JobScheduler uses std::thread; propagate the platform threads library to every consumer (a no-op
# on MSVC, links pthread on POSIX).
find_package(Threads REQUIRED)

target_include_directories(${PROJECT_NAME} INTERFACE
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)

enable_testing()
add_subdirectory(test)
