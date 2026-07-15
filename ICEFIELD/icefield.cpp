#include <AnimationComponent.h>
#include <ICEEngine.h>
#include <ICEMath.h>
#include <LightComponent.h>
#include <NativeScript.h>
#include <RenderComponent.h>
#include <TransformComponent.h>

using namespace ICE;

// Example per-entity game logic: spin the entity around Y. Attached via EntityHandle::script<>();
// the ScriptSystem instantiates and updates it every frame.
class Rotator : public NativeScript {
   public:
    void onUpdate(double dt) override {
        m_angle += dt * 45.0;  // 45 deg/sec, frame-rate independent
        registry()->getComponent<TransformComponent>(entity())->setRotationEulerDeg({0, (float) m_angle, 0});
    }

   private:
    double m_angle = 0.0;
};

int main() {
    std::filesystem::remove_all("IceField_project");

    ICEEngine engine({.title = "IceField", .width = 1280, .height = 720});
    engine.getWindow()->setSwapInterval(0);

    engine.getApi()->setClearColor(0.1f, 0.1f, 0.1f, 1.f);

    auto& project = engine.newProject("IceField_project");
    auto& scene = project.createScene("TestScene");
    auto hero = project.importModel("Adventurer", "ImportAssets/Adventurer.glb");

    engine.setUseRenderGraph(true);

    for (int i = 0; i < 10; ++i) {
        auto e = scene.create();
        e.add(TransformComponent({i - 5.f, 0, 0}, Eigen::Vector3f::Zero()));
        e.add(LightComponent(LightType::Point, {1, 1, 1}));
        e.add(RenderComponent(project.mesh("sphere"), project.material("base_mat")));
    }

    auto adv = scene.spawn(hero);
    adv.add(AnimationComponent{.currentAnimation = "Walk", .loop = true});
    adv.script<Rotator>();  

    // Camera faces -Z, and the scene sits at z = 0, so sit in front of it at +Z (this is what the
    // old backward(5)+up(5) produced) and pitch down to look at the group.
    scene.camera().setPosition({0, 5, 5}).pitch(-30);

    engine.run();
    return 0;
}
