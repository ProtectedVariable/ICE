#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include <IPlugin.h>
#include <Registry.h>
#include <System.h>

using namespace ICE;

// A component and a system defined entirely out-of-tree (here, in the test): the engine core knows
// nothing about either type. They reach the ECS only through the plugin seam.
struct HealthComponent {
    int hp = 100;
};

class HealthSystem : public System {
   public:
    void update(double) override {}
    std::vector<Signature> getSignatures(const ComponentManager& cm) const override {
        Signature signature;
        signature.set(cm.getComponentType<HealthComponent>());
        return {signature};
    }
};

class SamplePlugin : public IPlugin {
   public:
    const char* name() const override { return "SamplePlugin"; }
    void registerWith(PluginContext& ctx) override {
        // Register a gameplay system. (A plugin could also add asset loaders via ctx.asset_bank.)
        ctx.registry->addSystem(std::make_shared<HealthSystem>());
    }
};

// Acceptance (P11): an out-of-tree plugin registers a system and a component with no change to core.
TEST(PluginTest, RegistersSystemAndComponentWithoutCoreChange) {
    Registry registry;
    PluginContext ctx{&registry, nullptr};

    SamplePlugin plugin;
    plugin.registerWith(ctx);

    // The plugin's system is now part of the registry.
    ASSERT_NE(registry.getSystem<HealthSystem>(), nullptr);

    // The plugin's component works with zero registration ceremony (lazy registration, P2).
    Entity e = registry.createEntity();
    registry.addComponent(e, HealthComponent{42});
    ASSERT_TRUE(registry.entityHasComponent<HealthComponent>(e));
    EXPECT_EQ(registry.getComponent<HealthComponent>(e)->hp, 42);
}
