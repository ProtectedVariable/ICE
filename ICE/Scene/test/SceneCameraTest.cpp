#include <gtest/gtest.h>

#include <CameraComponent.h>
#include <PerspectiveCamera.h>
#include <Registry.h>
#include <Scene.h>
#include <SceneCamera.h>
#include <TransformComponent.h>

using namespace ICE;

namespace {
// Two view matrices are equal if every element matches within fp tolerance.
::testing::AssertionResult MatricesNear(const Eigen::Matrix4f& a, const Eigen::Matrix4f& b, float eps = 1e-4f) {
    if (a.isApprox(b, eps) || (a - b).cwiseAbs().maxCoeff() < eps) {
        return ::testing::AssertionSuccess();
    }
    return ::testing::AssertionFailure() << "matrices differ by " << (a - b).cwiseAbs().maxCoeff();
}
}  // namespace

// The whole behaviour-preservation guarantee: a free SceneCamera renders the same view as the
// scene-owned PerspectiveCamera it replaces, for the same pose.
TEST(SceneCameraTest, FreeCameraMatchesPerspectiveCamera) {
    SceneCamera sc;
    PerspectiveCamera ref(60.0, 16.0 / 9.0, 0.01, 10000.0);

    sc.setPosition({1.0f, 2.0f, 3.0f});
    ref.setPosition({1.0f, 2.0f, 3.0f});
    sc.pitch(-30.0f);
    ref.pitch(-30.0f);
    sc.yaw(45.0f);
    ref.yaw(45.0f);

    EXPECT_TRUE(MatricesNear(sc.lookThrough(), ref.lookThrough()));
    EXPECT_TRUE(MatricesNear(sc.getProjection(), ref.getProjection()));
    EXPECT_TRUE(sc.getPosition().isApprox(ref.getPosition()));
}

// A bound, unparented camera with a single-axis orientation (a pitch -- what the engine's cameras
// use in practice) produces the SAME view as the equivalent free/legacy camera. This is the
// behaviour-preservation guarantee for converting a scene camera to an entity camera: view =
// inverse(world) coincides with the legacy formula for a single axis.
TEST(SceneCameraTest, BoundUnparentedMatchesFreeCameraForPitch) {
    Registry reg;
    Entity e = reg.createEntity();
    reg.addComponent(e, CameraComponent{});  // default params == the free default
    TransformComponent t;
    t.setPosition({0.0f, 5.0f, 5.0f});
    t.setRotationEulerDeg({-30.0f, 0.0f, 0.0f});  // pitch only, as in the sample scenes
    reg.addComponent(e, t);

    SceneCamera bound;
    bound.bindEntity(&reg, e);

    PerspectiveCamera ref(60.0, 16.0 / 9.0, 0.01, 10000.0);
    ref.setPosition({0.0f, 5.0f, 5.0f});
    ref.setRotation({-30.0f, 0.0f, 0.0f});

    EXPECT_TRUE(MatricesNear(bound.lookThrough(), ref.lookThrough()));
    EXPECT_TRUE(bound.getPosition().isApprox(ref.getPosition(), 1e-4f));
}

// The bound view is a proper inverse of the entity's world transform: view * worldPoint maps the
// camera's own world position to the origin of view space.
TEST(SceneCameraTest, BoundViewIsInverseOfWorldTransform) {
    Registry reg;
    Entity e = reg.createEntity();
    reg.addComponent(e, CameraComponent{});
    TransformComponent t;
    t.setPosition({3.0f, -2.0f, 7.0f});
    t.setRotationEulerDeg({-30.0f, 45.0f, 10.0f});  // arbitrary multi-axis
    reg.addComponent(e, t);

    SceneCamera bound;
    bound.bindEntity(&reg, e);

    Eigen::Matrix4f view = bound.lookThrough();
    Eigen::Vector4f cam_in_view = view * Eigen::Vector4f(3.0f, -2.0f, 7.0f, 1.0f);
    EXPECT_TRUE(cam_in_view.head<3>().isApprox(Eigen::Vector3f::Zero(), 1e-4f));
}

// Acceptance: parenting the camera entity under a moving entity produces a follow camera with no
// special-case code. Moving the parent shifts the camera's world position and thus its view.
TEST(SceneCameraTest, ParentedCameraFollowsItsParent) {
    Scene scene("follow");
    auto reg = scene.getRegistry();

    EntityHandle parent = scene.create("rig");
    parent.add(TransformComponent({10.0f, 0.0f, 0.0f}, Eigen::Vector3f::Zero()));

    EntityHandle cam = scene.create("cam");
    cam.add(TransformComponent(Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()));  // local origin
    cam.add(CameraComponent{});
    scene.getGraph()->setParent(cam.id(), parent.id());
    scene.setActiveCamera(cam.id());

    // Propagate the parent's world matrix into the child's transform (what SceneGraphSystem does).
    auto parent_world = reg->getComponent<TransformComponent>(parent.id())->getWorldMatrix();
    reg->getComponent<TransformComponent>(cam.id())->updateParentMatrix(parent_world);

    // The camera sits at the parent's position, so the view translates the world by -parent.
    Eigen::Vector3f cam_world_pos = scene.cameraPtr()->getPosition();
    EXPECT_TRUE(cam_world_pos.isApprox(Eigen::Vector3f(10.0f, 0.0f, 0.0f), 1e-4f));

    // Move the rig; the camera follows with no camera-specific code.
    reg->getComponent<TransformComponent>(parent.id())->setPosition({-4.0f, 5.0f, 0.0f});
    parent_world = reg->getComponent<TransformComponent>(parent.id())->getWorldMatrix();
    reg->getComponent<TransformComponent>(cam.id())->updateParentMatrix(parent_world);

    EXPECT_TRUE(scene.cameraPtr()->getPosition().isApprox(Eigen::Vector3f(-4.0f, 5.0f, 0.0f), 1e-4f));
}

// Acceptance: selecting between two camera entities switches the view, through the same shared_ptr
// the render system holds (setActiveCamera rebinds in place).
TEST(SceneCameraTest, SwitchingActiveCameraSwitchesTheView) {
    Scene scene("switch");

    EntityHandle a = scene.create("camA");
    a.add(TransformComponent({0.0f, 0.0f, 5.0f}, Eigen::Vector3f::Zero()));
    a.add(CameraComponent{});

    EntityHandle b = scene.create("camB");
    b.add(TransformComponent({0.0f, 0.0f, -5.0f}, Eigen::Vector3f::Zero()));
    b.add(CameraComponent{});

    auto camera = scene.cameraPtr();  // the object the render system is handed

    scene.setActiveCamera(a.id());
    EXPECT_TRUE(camera->getPosition().isApprox(Eigen::Vector3f(0.0f, 0.0f, 5.0f), 1e-4f));

    scene.setActiveCamera(b.id());
    EXPECT_EQ(scene.cameraPtr(), camera);  // same object, rebound in place
    EXPECT_TRUE(camera->getPosition().isApprox(Eigen::Vector3f(0.0f, 0.0f, -5.0f), 1e-4f));
}

// The CameraComponent drives the projection: a different fov yields a different projection matrix.
TEST(SceneCameraTest, ProjectionComesFromTheCameraComponent) {
    Registry reg;
    Entity e = reg.createEntity();
    CameraComponent cc;
    cc.fov = 90.0f;
    reg.addComponent(e, cc);
    reg.addComponent(e, TransformComponent{});

    SceneCamera bound;
    bound.bindEntity(&reg, e);

    PerspectiveCamera ref90(90.0, 16.0 / 9.0, 0.01, 10000.0);
    EXPECT_TRUE(MatricesNear(bound.getProjection(), ref90.getProjection()));

    PerspectiveCamera ref60(60.0, 16.0 / 9.0, 0.01, 10000.0);
    EXPECT_FALSE(MatricesNear(bound.getProjection(), ref60.getProjection()));  // not the default
}

// Returning to the free camera (NULL_ENTITY) restores free behaviour.
TEST(SceneCameraTest, UnbindReturnsToFreeCamera) {
    Registry reg;
    Entity e = reg.createEntity();
    reg.addComponent(e, CameraComponent{});
    reg.addComponent(e, TransformComponent({7.0f, 0.0f, 0.0f}, Eigen::Vector3f::Zero()));

    SceneCamera sc;
    sc.bindEntity(&reg, e);
    EXPECT_TRUE(sc.isBound());
    EXPECT_TRUE(sc.getPosition().isApprox(Eigen::Vector3f(7.0f, 0.0f, 0.0f), 1e-4f));

    sc.bindEntity(nullptr, NULL_ENTITY);
    EXPECT_FALSE(sc.isBound());
    sc.setPosition({2.0f, 2.0f, 2.0f});
    EXPECT_TRUE(sc.getPosition().isApprox(Eigen::Vector3f(2.0f, 2.0f, 2.0f), 1e-4f));
}
