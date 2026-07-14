#include <gtest/gtest.h>

#include <memory>
#include <type_traits>

#include "GpuHandle.h"
#include "HandlePool.h"

using namespace ICE;

namespace {
struct TestTag {};
}  // namespace

// The typed GPU handles must be distinct types so a MeshHandle can't be used where a TextureHandle
// is expected.
static_assert(!std::is_same_v<MeshHandle, TextureHandle>, "GPU handle tags must be distinct");
static_assert(!std::is_same_v<MeshHandle, ShaderHandle>, "GPU handle tags must be distinct");

TEST(HandlePoolTest, InsertAndGet) {
    HandlePool<int, TestTag> pool;
    auto h = pool.insert(42);
    ASSERT_TRUE(h.valid());
    auto* p = pool.get(h);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(*p, 42);
    EXPECT_EQ(pool.size(), 1u);
}

TEST(HandlePoolTest, NullHandleResolvesToNull) {
    HandlePool<int, TestTag> pool;
    Handle<TestTag> null_h{};
    EXPECT_FALSE(null_h.valid());
    EXPECT_EQ(pool.get(null_h), nullptr);
}

TEST(HandlePoolTest, EraseFreesSlot) {
    HandlePool<int, TestTag> pool;
    auto h = pool.insert(7);
    EXPECT_TRUE(pool.erase(h));
    EXPECT_EQ(pool.get(h), nullptr);
    EXPECT_EQ(pool.size(), 0u);
    EXPECT_FALSE(pool.erase(h));  // double-erase is a no-op
}

// Acceptance criterion for P8: a stale handle (its slot freed and reused) does NOT resolve to the
// new occupant -- the generation mismatch catches the use-after-free.
TEST(HandlePoolTest, GenerationCatchesUseAfterFree) {
    HandlePool<int, TestTag> pool;
    auto ha = pool.insert(100);
    pool.erase(ha);
    auto hb = pool.insert(200);  // reuses ha's slot with a bumped generation

    EXPECT_EQ(hb.index, ha.index);            // same slot ...
    EXPECT_NE(hb.generation, ha.generation);  // ... different generation
    EXPECT_EQ(pool.get(ha), nullptr);         // stale handle: use-after-free caught
    ASSERT_NE(pool.get(hb), nullptr);
    EXPECT_EQ(*pool.get(hb), 200);            // and never aliases the new resource
}

TEST(HandlePoolTest, StableAddressesAcrossGrowth) {
    HandlePool<int, TestTag> pool;
    auto h0 = pool.insert(1);
    int* a0 = pool.get(h0);
    for (int i = 0; i < 1000; ++i) {
        pool.insert(i);  // force the backing store to grow
    }
    EXPECT_EQ(pool.get(h0), a0);  // an existing element's address is unchanged
    EXPECT_EQ(*pool.get(h0), 1);
}

TEST(HandlePoolTest, OwnsResourcesAndReleasesOnErase) {
    HandlePool<std::shared_ptr<int>, TestTag> pool;
    auto res = std::make_shared<int>(5);
    std::weak_ptr<int> weak = res;
    auto h = pool.insert(res);
    res.reset();
    EXPECT_FALSE(weak.expired());  // the pool keeps the resource alive
    pool.erase(h);
    EXPECT_TRUE(weak.expired());   // erase releases it
}
