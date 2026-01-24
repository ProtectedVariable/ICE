#define STB_IMAGE_IMPLEMENTATION

#include <AssetBank.h>
#include <gtest/gtest.h>

#include "MeshLoader.h"

using namespace ICE;

TEST(ModelLoaderTest, LoadFromObj) {
    auto mesh = MeshLoader().load({"cube.obj"});
    EXPECT_EQ(mesh->getVertices().size(), 36);
    EXPECT_EQ(mesh->getIndices().size(), 12);
}