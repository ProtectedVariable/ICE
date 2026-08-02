#include <gtest/gtest.h>

#include <type_traits>

#include "GpuHandle.h"

using namespace ICE;

// The typed GPU handles must be distinct types so a MeshHandle can't be used where a TextureHandle
// is expected. The generic HandlePool behaviour these are built on is covered by the container
// suite (ICE/Container/test), which is where HandlePool.h now lives.
static_assert(!std::is_same_v<MeshHandle, TextureHandle>, "GPU handle tags must be distinct");
static_assert(!std::is_same_v<MeshHandle, ShaderHandle>, "GPU handle tags must be distinct");
static_assert(!std::is_same_v<TextureHandle, ShaderHandle>, "GPU handle tags must be distinct");

TEST(GpuHandleTest, DefaultConstructedHandlesAreInvalid) {
    EXPECT_FALSE(MeshHandle{}.valid());
    EXPECT_FALSE(TextureHandle{}.valid());
    EXPECT_FALSE(ShaderHandle{}.valid());
}
