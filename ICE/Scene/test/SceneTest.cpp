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

#endif //ICE_SCENETEST_H
