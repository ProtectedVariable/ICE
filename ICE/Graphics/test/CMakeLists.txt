cmake_minimum_required(VERSION 3.19)
project(graphics-tests)

message(STATUS "Building ${PROJECT_NAME} suite")
include(CTest)

# The generic HandlePool suite moved to ICE/Container/test along with HandlePool.h itself; what
# remains here is the graphics-specific tagging built on top of it.
add_executable(GpuHandleTestSuite
        GpuHandleTest.cpp
)

add_test(NAME GpuHandleTestSuite
        COMMAND GpuHandleTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:GpuHandleTestSuite>)

target_link_libraries(GpuHandleTestSuite
        PRIVATE
        gtest_main
        graphics
)

# RenderGraph.h transitively pulls in the graphics headers, so this suite links the graphics
# library for the include paths. It exercises only compile-time graph logic (no GL is invoked).
add_executable(RenderGraphTestSuite
        RenderGraphTest.cpp
)

add_test(NAME RenderGraphTestSuite
        COMMAND RenderGraphTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:RenderGraphTestSuite>)

target_link_libraries(RenderGraphTestSuite
        PRIVATE
        gtest_main
        graphics
)

# The public pass/feature seam (typed handles, feature registration, setup/execute). Exercises
# compile-time graph logic plus a stub framebuffer -- no GL is invoked.
add_executable(RenderFeatureTestSuite
        RenderFeatureTest.cpp
)

add_test(NAME RenderFeatureTestSuite
        COMMAND RenderFeatureTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:RenderFeatureTestSuite>)

target_link_libraries(RenderFeatureTestSuite
        PRIVATE
        gtest_main
        graphics
)

# Compile-once + resource pooling + descriptor-based allocation + the pass draw seam (T5). Uses a
# counting stub factory, so allocation counts are observable without GL.
add_executable(RenderGraphPoolingTestSuite
        RenderGraphPoolingTest.cpp
)

add_test(NAME RenderGraphPoolingTestSuite
        COMMAND RenderGraphPoolingTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:RenderGraphPoolingTestSuite>)

target_link_libraries(RenderGraphPoolingTestSuite
        PRIVATE
        gtest_main
        graphics
)

# Sort-key packing/determinism (RenderCommand.h pulls in the graphics headers, so link graphics).
add_executable(SortKeyTestSuite
        SortKeyTest.cpp
)

add_test(NAME SortKeyTestSuite
        COMMAND SortKeyTestSuite
        WORKING_DIRECTORY $<TARGET_FILE_DIR:SortKeyTestSuite>)

target_link_libraries(SortKeyTestSuite
        PRIVATE
        gtest_main
        graphics
)
