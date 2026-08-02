#include <Camera.h>
#include <OrthographicCamera.h>
#include <PerspectiveCamera.h>
#include <gtest/gtest.h>

#include <Eigen/Dense>

#include "AABB.h"
#include "ICEMath.h"

using namespace ICE;

class FrustumCullingTest : public ::testing::Test {
   protected:
    std::shared_ptr<Camera> createPerspectiveMatrix(float fov, float aspect, float near, float far) {
        return std::make_shared<PerspectiveCamera>(fov, aspect, near, far);
    }
};

// Test extractFrustumPlanes
TEST_F(FrustumCullingTest, ExtractFrustumPlanes_StandardPerspective) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    // Check that all plane normals are normalized
    for (int i = 0; i < 6; i++) {
        float norm = frustum.planes[i].normal.norm();
        EXPECT_NEAR(norm, 1.0f, 1e-5) << "Plane " << i << " normal is not normalized";
    }
}

TEST_F(FrustumCullingTest, ExtractFrustumPlanes_AbsNormalComputation) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    // Check that absNormal is the absolute value of normal
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 3; j++) {
            EXPECT_NEAR(frustum.planes[i].absNormal(j), fabsf(frustum.planes[i].normal(j)), 1e-5)
                << "Plane " << i << " absNormal mismatch at component " << j;
        }
    }
}

// Test isAABBInFrustum with AABB object
TEST_F(FrustumCullingTest, AABBInFrustum_CompletelyInside) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    // Create a small AABB at the origin (center of view)
    AABB box(std::vector<Eigen::Vector3f>{Eigen::Vector3f(-0.5f, -0.5f, -0.5f), Eigen::Vector3f(0.5f, 0.5f, 0.5f)});

    EXPECT_TRUE(isAABBInFrustum(frustum, box));
}

TEST_F(FrustumCullingTest, AABBInFrustum_PartiallyInside) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    // Create an AABB that straddles the near plane
    AABB box(std::vector<Eigen::Vector3f>{Eigen::Vector3f(-1.0f, -1.0f, -0.05f), Eigen::Vector3f(1.0f, 1.0f, -0.5f)});

    EXPECT_TRUE(isAABBInFrustum(frustum, box));
}

TEST_F(FrustumCullingTest, AABBInFrustum_FarPlane) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    // Create an AABB beyond the far plane
    AABB box(std::vector<Eigen::Vector3f>{Eigen::Vector3f(-1.0f, -1.0f, -101.0f), Eigen::Vector3f(1.0f, 1.0f, -102.0f)});

    EXPECT_FALSE(isAABBInFrustum(frustum, box));
}

TEST_F(FrustumCullingTest, AABBInFrustum_OutsideLeftPlane) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    // Create an AABB to the far left, outside the frustum
    AABB box(std::vector<Eigen::Vector3f>{Eigen::Vector3f(-100.0f, -1.0f, -5.0f), Eigen::Vector3f(-50.0f, 1.0f, -3.0f)});

    EXPECT_FALSE(isAABBInFrustum(frustum, box));
}

// Test isAABBInFrustum with center and extents
TEST_F(FrustumCullingTest, CenterExtentsInFrustum_Inside) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    Eigen::Vector3f aabb_center(0, 0, -10);
    Eigen::Vector3f aabb_extents(0.5f, 0.5f, 0.5f);

    EXPECT_TRUE(isAABBInFrustum(frustum, aabb_center, aabb_extents));
}

TEST_F(FrustumCullingTest, CenterExtentsInFrustum_Outside) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    Eigen::Vector3f aabb_center(-500, 0, 0);
    Eigen::Vector3f aabb_extents(1.0f, 1.0f, 1.0f);

    EXPECT_FALSE(isAABBInFrustum(frustum, aabb_center, aabb_extents));
}

TEST_F(FrustumCullingTest, CenterExtentsInFrustum_BehindCamera) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    Eigen::Vector3f aabb_center(0, 0, 10);
    Eigen::Vector3f aabb_extents(1.0f, 1.0f, 1.0f);

    EXPECT_FALSE(isAABBInFrustum(frustum, aabb_center, aabb_extents));
}

TEST_F(FrustumCullingTest, CenterExtentsInFrustum_OutsideRightPlane) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    Eigen::Vector3f aabb_center(500, 0, 0);
    Eigen::Vector3f aabb_extents(1.0f, 1.0f, 1.0f);

    EXPECT_FALSE(isAABBInFrustum(frustum, aabb_center, aabb_extents));
}

TEST_F(FrustumCullingTest, CenterExtentsInFrustum_OutsideTopPlane) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    Eigen::Vector3f aabb_center(0, 500, 0);
    Eigen::Vector3f aabb_extents(1.0f, 1.0f, 1.0f);

    EXPECT_FALSE(isAABBInFrustum(frustum, aabb_center, aabb_extents));
}

TEST_F(FrustumCullingTest, CenterExtentsInFrustum_PartiallyVisible) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    Eigen::Vector3f aabb_center(1.0f, 1.0f, -5.0f);
    Eigen::Vector3f aabb_extents(2.0f, 2.0f, 2.0f);

    EXPECT_TRUE(isAABBInFrustum(frustum, aabb_center, aabb_extents));
}

// Edge case: very small box
TEST_F(FrustumCullingTest, AABBInFrustum_VerySmallBox) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    AABB box(std::vector<Eigen::Vector3f>{Eigen::Vector3f(0, 0, -10), Eigen::Vector3f(0.001f, 0.001f, -10.001f)});

    EXPECT_TRUE(isAABBInFrustum(frustum, box));
}

// Edge case: very large box containing camera
TEST_F(FrustumCullingTest, AABBInFrustum_VeryLargeBoxContainingCamera) {

    auto camera = createPerspectiveMatrix(M_PI / 4.0f, 16.0f / 9.0f, 0.1f, 100.0f);
    Eigen::Matrix4f pv = camera->getProjection() * camera->lookThrough();

    Frustum frustum = extractFrustumPlanes(pv);

    AABB box(Eigen::Vector3f(-1000, -1000, -1000), Eigen::Vector3f(1000, 1000, 1000));

    EXPECT_TRUE(isAABBInFrustum(frustum, box));
}
