#include <AssetBank.h>
#include <NoneGraphicsFactory.h>
#include <gtest/gtest.h>

#include "ModelLoader.h"

using namespace ICE;

TEST(ModelLoaderTest, LoadFromObj) {
    auto gr_f = std::make_shared<NoneGraphicsFactory>();
    AssetBank bank(gr_f);
    auto mesh = ModelLoader(gr_f, bank).load({"cube.obj"});
    EXPECT_EQ(mesh->getMeshes().at(0)->getVertices().size(), 24);
    EXPECT_EQ(mesh->getMeshes().at(0)->getIndices().size(), 12);
}