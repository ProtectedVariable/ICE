#include <Camera.h>
#include <OrthographicCamera.h>
#include <PerspectiveCamera.h>
#include <gtest/gtest.h>

#include <Eigen/Dense>

#include "AABB.h"
#include "ICEMath.h"

using namespace ICE;

class FrustumCullingTest : public ::testing::Test {
  
};
// Test AABB class
TEST_F(FrustumCullingTest, AABB_Construction) {
    Eigen::Vector3f min(0, 1, 2);
    Eigen::Vector3f max(3, 4, 5);

    AABB box(min, max);

    EXPECT_EQ(box.getMin(), min);
    EXPECT_EQ(box.getMax(), max);
}

TEST_F(FrustumCullingTest, AABB_CenterAndExtent) {
    Eigen::Vector3f min(0, 0, 0);
    Eigen::Vector3f max(2, 4, 6);

    AABB box(min, max);

    Eigen::Vector3f expectedCenter(1, 2, 3);
    Eigen::Vector3f expectedExtent(1, 2, 3);

    EXPECT_EQ(box.getCenter(), expectedCenter);
    EXPECT_EQ(box.getExtent(), expectedExtent);
}

TEST_F(FrustumCullingTest, AABB_Overlaps) {
    AABB box1(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(2, 2, 2));
    AABB box2(Eigen::Vector3f(1, 1, 1), Eigen::Vector3f(3, 3, 3));
    AABB box3(Eigen::Vector3f(5, 5, 5), Eigen::Vector3f(7, 7, 7));

    EXPECT_TRUE(box1.overlaps(box2));
    EXPECT_TRUE(box2.overlaps(box1));
    EXPECT_FALSE(box1.overlaps(box3));
    EXPECT_FALSE(box3.overlaps(box1));
}

TEST_F(FrustumCullingTest, AABB_Contains) {
    AABB box(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(2, 2, 2));

    EXPECT_TRUE(box.contains(Eigen::Vector3f(1, 1, 1)));
    EXPECT_TRUE(box.contains(Eigen::Vector3f(0, 0, 0)));
    EXPECT_TRUE(box.contains(Eigen::Vector3f(2, 2, 2)));
    EXPECT_FALSE(box.contains(Eigen::Vector3f(3, 3, 3)));
    EXPECT_FALSE(box.contains(Eigen::Vector3f(-1, 0, 0)));
}

TEST_F(FrustumCullingTest, AABB_ScaledBy) {
    AABB box(Eigen::Vector3f(1, 2, 3), Eigen::Vector3f(4, 6, 9));
    AABB scaled = box.scaledBy(Eigen::Vector3f(2, 2, 2));

    EXPECT_EQ(scaled.getMin(), Eigen::Vector3f(2, 4, 6));
    EXPECT_EQ(scaled.getMax(), Eigen::Vector3f(8, 12, 18));
}

TEST_F(FrustumCullingTest, AABB_TranslatedBy) {
    AABB box(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(2, 2, 2));
    AABB translated = box.translatedBy(Eigen::Vector3f(5, 10, 15));

    EXPECT_EQ(translated.getMin(), Eigen::Vector3f(5, 10, 15));
    EXPECT_EQ(translated.getMax(), Eigen::Vector3f(7, 12, 17));
}

TEST_F(FrustumCullingTest, AABB_UnionWith) {
    AABB box1(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(2, 2, 2));
    AABB box2(Eigen::Vector3f(1, 1, 1), Eigen::Vector3f(5, 5, 5));

    AABB unified = box1.unionWith(box2);

    EXPECT_EQ(unified.getMin(), Eigen::Vector3f(0, 0, 0));
    EXPECT_EQ(unified.getMax(), Eigen::Vector3f(5, 5, 5));
}

TEST_F(FrustumCullingTest, AABB_Volume) {
    AABB box(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(2, 3, 4));

    EXPECT_FLOAT_EQ(box.getVolume(), 24.0f);
}

TEST_F(FrustumCullingTest, AABB_ConstructFromPoints) {
    std::vector<Eigen::Vector3f> points = {Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(5, 2, 3), Eigen::Vector3f(1, 8, 2), Eigen::Vector3f(3, 1, 6)};

    AABB box(points);

    EXPECT_EQ(box.getMin(), Eigen::Vector3f(0, 0, 0));
    EXPECT_EQ(box.getMax(), Eigen::Vector3f(5, 8, 6));
}