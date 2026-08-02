#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include <NativeScript.h>
#include <NativeScriptComponent.h>
#include <Registry.h>
#include <ScriptSystem.h>
#include <TransformComponent.h>

using namespace ICE;

namespace {
// Captures what a script saw through its T3 behaviour context, so a test can assert on it after the
// ScriptSystem has driven the (system-owned) instance.
struct Probe {
    int creates = 0;
    int updates = 0;
    int destroys = 0;
    double last_time = -1.0;
    bool self_valid = false;
    bool transform_seen = false;
    Scene* scene = nullptr;
    InputManager* input = nullptr;
    bool alive_in_update = false;
};

// Reads its context every frame and moves the entity through transform() -- no registry plumbing.
// Constructor args flow through EntityHandle::script<T>()/NativeScriptComponent::bind<T>().
class Probed : public NativeScript {
   public:
    explicit Probed(Probe* p) : m_p(p) {}
    void onCreate() override { m_p->creates++; }
    void onUpdate(double) override {
        m_p->updates++;
        m_p->last_time = time();
        m_p->self_valid = self().valid();
        m_p->scene = scene();
        m_p->input = input();
        if (auto* t = transform()) {
            m_p->transform_seen = true;
            t->position().x() += 1.0f;  // observable side effect, reached via the context
        }
    }
    void onDestroy() override { m_p->destroys++; }

   private:
    Probe* m_p;
};

// Destroys itself from within onUpdate: proves destroy() defers teardown to the end of the frame.
class SelfDestroyer : public NativeScript {
   public:
    explicit SelfDestroyer(Probe* p) : m_p(p) {}
    void onUpdate(double) override {
        m_p->updates++;
        m_p->alive_in_update = self().valid();           // still live mid-update
        m_p->transform_seen = (transform() != nullptr);  // components still present mid-update
        destroy();
    }
    void onDestroy() override { m_p->destroys++; }

   private:
    Probe* m_p;
};

// A registry driven by a ScriptSystem. The scene/input pointers are only threaded through to
// scripts (never dereferenced by the system), so tests pass sentinels to prove they arrive.
std::shared_ptr<Registry> makeRegistry(Scene* scene, InputManager* input) {
    auto reg = std::make_shared<Registry>();
    reg->addSystem(std::make_shared<ScriptSystem>(reg, scene, input));
    return reg;
}

Entity attachScript(const std::shared_ptr<Registry>& reg, NativeScriptComponent nsc) {
    Entity e = reg->createEntity();
    reg->addComponent(e, TransformComponent());
    reg->addComponent(e, std::move(nsc));  // signature now matches -> onEntityAdded -> onCreate
    return e;
}
}  // namespace

// The context is injected and the lifecycle driven; time() accumulates and transform() edits land.
TEST(ScriptSystemTest, InjectsContextAndDrivesLifecycle) {
    // Bogus but distinct pointers: the system only forwards them to scene()/input(), never
    // dereferences them, so their targets need not exist.
    Scene* scene_sentinel = reinterpret_cast<Scene*>(0x100);
    InputManager* input_sentinel = reinterpret_cast<InputManager*>(0x200);
    auto reg = makeRegistry(scene_sentinel, input_sentinel);

    Probe probe;
    NativeScriptComponent nsc;
    nsc.bind<Probed>(&probe);
    Entity e = attachScript(reg, std::move(nsc));

    EXPECT_EQ(probe.creates, 1);  // onCreate fired when the script component was attached
    EXPECT_EQ(probe.updates, 0);

    reg->updateSystems(0.5);
    EXPECT_EQ(probe.updates, 1);
    EXPECT_TRUE(probe.self_valid);
    EXPECT_TRUE(probe.transform_seen);
    EXPECT_EQ(probe.scene, scene_sentinel);
    EXPECT_EQ(probe.input, input_sentinel);
    EXPECT_DOUBLE_EQ(probe.last_time, 0.5);
    EXPECT_FLOAT_EQ(reg->getComponent<TransformComponent>(e)->getPosition().x(), 1.0f);

    reg->updateSystems(0.25);
    EXPECT_EQ(probe.updates, 2);
    EXPECT_DOUBLE_EQ(probe.last_time, 0.75);  // time keeps accumulating across frames
    EXPECT_FLOAT_EQ(reg->getComponent<TransformComponent>(e)->getPosition().x(), 2.0f);
}

// destroy() called from onUpdate: the script finishes with a live entity, teardown happens after
// the pass, and the entity stays gone (no resurrection, no double onDestroy).
TEST(ScriptSystemTest, DestroyDefersTeardownToFrameEnd) {
    auto reg = makeRegistry(nullptr, nullptr);

    Probe probe;
    NativeScriptComponent nsc;
    nsc.bind<SelfDestroyer>(&probe);
    Entity e = attachScript(reg, std::move(nsc));
    EXPECT_TRUE(reg->isAlive(e));

    reg->updateSystems(0.016);
    EXPECT_EQ(probe.updates, 1);
    EXPECT_TRUE(probe.alive_in_update);   // entity was live throughout its own onUpdate
    EXPECT_TRUE(probe.transform_seen);    // ...and its components were still present
    EXPECT_EQ(probe.destroys, 1);         // onDestroy fired exactly once, after the pass
    EXPECT_FALSE(reg->isAlive(e));        // removed by frame end

    reg->updateSystems(0.016);
    EXPECT_EQ(probe.updates, 1);          // no further onUpdate (dropped from the system)
    EXPECT_EQ(probe.destroys, 1);         // no double teardown
}

// scene()/input() are null when nothing is wired (a headless scene), not left dangling.
TEST(ScriptSystemTest, NullContextWhenUnwired) {
    auto reg = makeRegistry(nullptr, nullptr);

    Probe probe;
    NativeScriptComponent nsc;
    nsc.bind<Probed>(&probe);
    attachScript(reg, std::move(nsc));

    reg->updateSystems(0.016);
    EXPECT_EQ(probe.scene, nullptr);
    EXPECT_EQ(probe.input, nullptr);
    EXPECT_TRUE(probe.self_valid);
}
