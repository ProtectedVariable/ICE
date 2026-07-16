#include <AnimationComponent.h>
#include <ICEEngine.h>
#include <ICEMath.h>
#include <InputManager.h>
#include <LightComponent.h>
#include <NativeScript.h>
#include <RenderComponent.h>
#include <TransformComponent.h>

using namespace ICE;

// Example per-entity game logic exercising the T3 behaviour context: it spins the entity from the
// accumulated time() and drives it around the XZ plane from input(), reaching everything through
// the context (transform(), input(), time()) -- no registry()->getComponent() plumbing. Attached
// via EntityHandle::script<>(); the ScriptSystem instantiates, injects context, and updates it.
class PlayerController : public NativeScript {
   public:
    void onUpdate(double dt) override {
        auto* t = transform();
        if (!t) return;

        // Frame-rate independent spin straight off accumulated time.
        t->setRotationEulerDeg({0, (float) (time() * 45.0), 0});

        // WASD translates in the XZ plane (no-op until an input service is wired).
        Eigen::Vector3f move = Eigen::Vector3f::Zero();
        if (auto* in = input()) {
            if (in->isKeyDown(Key::KEY_W)) move.z() -= 1.f;
            if (in->isKeyDown(Key::KEY_S)) move.z() += 1.f;
            if (in->isKeyDown(Key::KEY_A)) move.x() -= 1.f;
            if (in->isKeyDown(Key::KEY_D)) move.x() += 1.f;
        }
        if (!move.isZero()) {
            t->setPosition(t->getPosition() + move.normalized() * (float) (dt * 3.0));
        }
    }
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
    adv.script<PlayerController>();

    // Camera faces -Z, and the scene sits at z = 0, so sit in front of it at +Z (this is what the
    // old backward(5)+up(5) produced) and pitch down to look at the group.
    scene.camera().setPosition({0, 5, 5}).pitch(-30);

    engine.run();
    return 0;
}
