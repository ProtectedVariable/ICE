#ifndef ICE_ECSTEST_H
#define ICE_ECSTEST_H
#include <gtest/gtest.h>
#include <Registry.h>
#include <TransformComponent.h>
#include <RenderComponent.h>

TEST(ECSTest, FirstEntityIs1)
{
	Registry reg = Registry();
	Entity e = reg.createEntity();
	ASSERT_EQ(e, 1);
}

TEST(ECSTest, Create100Entities)
{
	Registry reg = Registry();
	Entity e;
	for(size_t i = 0; i < 100; i++) {
		e = reg.createEntity();
	}
	ASSERT_EQ(e, 100);
}

TEST(ECSTest, ReuseReleasedId)
{
	Registry reg = Registry();
	Entity e;
	for(size_t i = 0; i < 100; i++) {
		e = reg.createEntity();
	}
	reg.removeEntity(42);
	e = reg.createEntity();
	ASSERT_EQ(e, 42);
	e = reg.createEntity();
	ASSERT_EQ(e, 101);
}

TEST(ECSTest, ComponentAdded)
{
	Registry reg = Registry();
	Entity e = reg.createEntity();
	reg.addComponent(e, TransformComponent());
	ASSERT_TRUE(reg.entityHasComponent<TransformComponent>(e));
}

TEST(ECSTest, ComponentRemoveAndAdd)
{
	Registry reg = Registry();
	Entity e = reg.createEntity();
	reg.addComponent(e, TransformComponent());
	reg.removeComponent<TransformComponent>(e);
	reg.addComponent(e, TransformComponent());
	ASSERT_TRUE(reg.entityHasComponent<TransformComponent>(e));
}

TEST(ECSTest, ComponentRemoved)
{
	Registry reg = Registry();
	Entity e = reg.createEntity();
	reg.addComponent(e, TransformComponent());
	reg.removeComponent<TransformComponent>(e);
	ASSERT_FALSE(reg.entityHasComponent<TransformComponent>(e));
}


TEST(ECSTest, ComponentMultipleAdded)
{
	Registry reg = Registry();
	Entity e = reg.createEntity();
	reg.addComponent(e, TransformComponent());
	reg.addComponent(e, RenderComponent());
	ASSERT_TRUE(reg.entityHasComponent<TransformComponent>(e));
	ASSERT_TRUE(reg.entityHasComponent<RenderComponent>(e));
}

#endif //ICE_ECSTEST_H
