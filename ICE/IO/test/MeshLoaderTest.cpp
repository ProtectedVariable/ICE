#include <gtest/gtest.h>

#include "MeshLoader.h"
#include <NoneGraphicsFactory.h>

using namespace ICE;

TEST(MeshLoaderTest, LoadFromObj) {
    auto gr_f = std::make_shared<NoneGraphicsFactory>();
    auto mesh = MeshLoader(gr_f).load({"cube.obj"});
    EXPECT_EQ(mesh->getVertices().size(), 24);
    EXPECT_EQ(mesh->getIndices().size(), 12);
}