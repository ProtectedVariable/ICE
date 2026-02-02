#include <ICEHelpers.h>
#include <ICEEngine.h>
#include <gtest/gtest.h>

using namespace ICE;

class HelpersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup if needed
    }
};

TEST_F(HelpersTest, EntityHelperBasicUsage) {
    // Create a mock registry
    auto registry = std::make_shared<Registry>();
    
    // Create an entity
    Entity entity = registry->getEntityManager().createEntity();
    
    // Add a transform component
    registry->addComponent<TransformComponent>(entity, 
        TransformComponent({0, 0, 0}, {0, 0, 0}, {1, 1, 1}));
    
    // Use EntityHelper
    EntityHelper helper(entity, registry);
    
    // Test validity
    EXPECT_TRUE(helper.isValid());
    EXPECT_EQ(helper.id(), entity);
    
    // Test component access
    auto transform = helper.transform();
    ASSERT_NE(transform, nullptr);
    
    // Test position modification
    transform->setPosition({1, 2, 3});
    EXPECT_EQ(transform->getPosition().x(), 1.0f);
    EXPECT_EQ(transform->getPosition().y(), 2.0f);
    EXPECT_EQ(transform->getPosition().z(), 3.0f);
}

TEST_F(HelpersTest, EntityHelperHasComponent) {
    auto registry = std::make_shared<Registry>();
    Entity entity = registry->getEntityManager().createEntity();
    
    EntityHelper helper(entity, registry);
    
    // Should not have transform initially
    EXPECT_FALSE(helper.hasComponent<TransformComponent>());
    
    // Add component
    helper.addComponent<TransformComponent>(
        TransformComponent({0, 0, 0}, {0, 0, 0}, {1, 1, 1}));
    
    // Should have it now
    EXPECT_TRUE(helper.hasComponent<TransformComponent>());
}

TEST_F(HelpersTest, EntityHelperInvalidEntity) {
    auto registry = std::make_shared<Registry>();
    
    // Create helper with invalid entity (0)
    EntityHelper helper(0, registry);
    
    // Should still be "valid" (has registry)
    EXPECT_TRUE(helper.isValid());
    EXPECT_EQ(helper.id(), 0);
}

TEST_F(HelpersTest, EngineHelperNullSafety) {
    ICEEngine engine;
    
    // Engine has no project yet
    EXPECT_FALSE(EngineHelper::isEngineReady(engine));
    
    // Should return nullptr safely
    EXPECT_EQ(EngineHelper::getRegistry(engine), nullptr);
    EXPECT_EQ(EngineHelper::getRegistryPtr(engine), nullptr);
    EXPECT_EQ(EngineHelper::getAssetBank(engine), nullptr);
    EXPECT_EQ(EngineHelper::getCurrentScene(engine), nullptr);
    EXPECT_EQ(EngineHelper::getCamera(engine), nullptr);
}
