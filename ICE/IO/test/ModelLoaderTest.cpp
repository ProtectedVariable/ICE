#define STB_IMAGE_IMPLEMENTATION

#include <AssetBank.h>
#include <gtest/gtest.h>

#include "ModelLoader.h"

using namespace ICE;

TEST(ModelLoaderTest, LoadFromObj) {
    AssetBank bank;
    auto mesh = ModelLoader(bank).load({"cube.obj"});
    EXPECT_EQ(mesh->getMeshes().at(0)->getVertices().size(), 36);
    EXPECT_EQ(mesh->getMeshes().at(0)->getIndices().size(), 12);
}