#ifndef ICE_ECSTEST_H
#define ICE_ECSTEST_H
#include <gtest/gtest.h>
#include <Registry.h>
#include <TransformComponent.h>
#include <RenderComponent.h>

using namespace ICE;

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

// A component type the ECS core has never heard of: not in any registration list, no header
// included by Registry. Exercises P2's lazy, implicit registration.
struct CustomTestComponent {
	int value = 0;
};

// entityHasComponent<T> must be safe for a type that has never been added anywhere -- it goes
// through the const getComponentType path (same one System::getSignatures uses) which must not
// require the component's storage to exist.
TEST(ECSTest, HasComponentFalseForUntouchedType)
{
	Registry reg = Registry();
	Entity e = reg.createEntity();
	ASSERT_FALSE(reg.entityHasComponent<CustomTestComponent>(e));
}

// Add / query / remove a brand-new component type with zero registration calls.
TEST(ECSTest, CustomComponentRegistersOnFirstUse)
{
	Registry reg = Registry();
	Entity e = reg.createEntity();
	reg.addComponent(e, CustomTestComponent{42});
	ASSERT_TRUE(reg.entityHasComponent<CustomTestComponent>(e));
	ASSERT_EQ(reg.getComponent<CustomTestComponent>(e)->value, 42);
	reg.removeComponent<CustomTestComponent>(e);
	ASSERT_FALSE(reg.entityHasComponent<CustomTestComponent>(e));
}

// Iterating a custom type mixed with a built-in one works without any registration step.
TEST(ECSTest, EachOverCustomComponent)
{
	Registry reg = Registry();
	Entity e1 = reg.createEntity();
	Entity e2 = reg.createEntity();
	reg.addComponent(e1, CustomTestComponent{7});
	reg.addComponent(e1, TransformComponent());
	reg.addComponent(e2, CustomTestComponent{9});  // no TransformComponent

	int matched = 0;
	int sum = 0;
	reg.each<CustomTestComponent, TransformComponent>([&](Entity, CustomTestComponent& c, TransformComponent&) {
		matched++;
		sum += c.value;
	});
	ASSERT_EQ(matched, 1);  // only e1 has both
	ASSERT_EQ(sum, 7);
}

// A component pointer must survive add/remove of OTHER entities' components. Under the old
// vector-backed storage the growth below would reallocate (and the removals would swap-move the
// tail), leaving `pa` dangling; the stable pool keeps it valid.
TEST(ECSTest, ComponentPointerStableAcrossInsertAndErase)
{
	Registry reg = Registry();
	Entity a = reg.createEntity();
	reg.addComponent(a, CustomTestComponent{100});
	CustomTestComponent* pa = reg.getComponent<CustomTestComponent>(a);

	// Force many insertions (growth) of the same component type on other entities.
	std::vector<Entity> others;
	for (int i = 0; i < 1000; i++) {
		Entity e = reg.createEntity();
		reg.addComponent(e, CustomTestComponent{i});
		others.push_back(e);
	}
	ASSERT_EQ(pa, reg.getComponent<CustomTestComponent>(a));  // same address after growth
	ASSERT_EQ(pa->value, 100);

	// Removing the other entities' components must not move or invalidate A's component.
	for (Entity e : others) {
		reg.removeComponent<CustomTestComponent>(e);
	}
	ASSERT_EQ(pa, reg.getComponent<CustomTestComponent>(a));
	ASSERT_EQ(pa->value, 100);

	// Writes through the cached pointer are still observed.
	pa->value = 7;
	ASSERT_EQ(reg.getComponent<CustomTestComponent>(a)->value, 7);
}

#endif //ICE_ECSTEST_H
