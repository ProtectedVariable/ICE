#ifndef ICE_SCENETEST_H
#define ICE_SCENETEST_H
#include <gtest/gtest.h>

TEST(SceneTest, SceneCreated)
{
	Scene scene = Scene("test");
	ASSERT_EQ(scene.getName(), "test");
}

TEST(SceneTest, SceneAddEntity)
{
	Scene scene = Scene("test");
	Entity e = scene.createEntity();
	ASSERT_EQ(e, 1);
	auto ent = scene.getEntities();
	ASSERT_EQ(ent.size(), 1);
	ASSERT_EQ(ent[0], e);
	Entity e2 = scene.createEntity();
	ASSERT_EQ(e2, 2);
	ASSERT_EQ(scene.getAlias(e2), "object");
}

TEST(SceneTest, SceneRemoveEntity)
{
	Scene scene = Scene("test");
	Entity e = scene.createEntity();
	Entity e2 = scene.createEntity();
	scene.removeEntity(e);
	auto ent = scene.getEntities();
	ASSERT_EQ(ent.size(), 1);
	ASSERT_EQ(ent[0], e2);
}

TEST(SceneTest, SceneRenameEntity)
{
	Scene scene = Scene("test");
	Entity e = scene.createEntity();
	scene.setAlias(e, "player");
	ASSERT_EQ(scene.getAlias(e), "player");
}

#endif //ICE_SCENETEST_H
