#ifndef ICE_SCENETEST_H
#define ICE_SCENETEST_H
#include <gtest/gtest.h>
#include <Scene.h>

using namespace ICE;

TEST(SceneTest, SceneCreated)
{
	Scene scene = Scene("test");
	ASSERT_EQ(scene.getName(), "test");
}

TEST(SceneTest, SceneRenameEntity)
{
	Scene scene = Scene("test");
	Entity e = scene.createEntity();
	scene.setAlias(e, "player");
	ASSERT_EQ(scene.getAlias(e), "player");
}

TEST(SceneTest, SceneRemoveEntityRemovesWholeSubtree)
{
	Scene scene = Scene("test");
	Entity parent = scene.createEntity();
	Entity child = scene.createEntity();
	Entity grandchild = scene.createEntity();
	scene.getGraph()->setParent(child, parent);
	scene.getGraph()->setParent(grandchild, child);

	scene.removeEntity(parent);

	ASSERT_FALSE(scene.hasEntity(parent));
	ASSERT_FALSE(scene.hasEntity(child));
	ASSERT_FALSE(scene.hasEntity(grandchild));
	ASSERT_EQ(scene.getGraph()->getRoot()->children.size(), 0);
	ASSERT_EQ(scene.getAlias(child), "");
}

TEST(SceneTest, SceneRemoveEntityKeepsSiblings)
{
	Scene scene = Scene("test");
	Entity parent = scene.createEntity();
	Entity child = scene.createEntity();
	Entity sibling = scene.createEntity();
	scene.getGraph()->setParent(child, parent);

	scene.removeEntity(parent);

	ASSERT_TRUE(scene.hasEntity(sibling));
	ASSERT_EQ(scene.getGraph()->getRoot()->children.size(), 1);
	ASSERT_EQ(scene.getGraph()->getRoot()->children[0]->entity, sibling);
}

#endif //ICE_SCENETEST_H
