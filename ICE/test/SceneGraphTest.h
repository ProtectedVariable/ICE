#ifndef ICE_SCENEGRAPHTEST_H
#define ICE_SCENEGRAPHTEST_H
#include <gtest/gtest.h>
#include <Scene/Scene.h>

TEST(SceneGraphTest, SceneGraphCreated)
{
	SceneGraph sg = SceneGraph();
	ASSERT_EQ(sg.root.entity, 0);
}

TEST(SceneGraphTest, SceneGraphAddEntity)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	ASSERT_EQ(sg.root.children[0]->entity, 1);
}

TEST(SceneGraphTest, SceneGraphSetParent)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	sg.addEntity(2);
	ASSERT_EQ(sg.root.children[1]->entity, 2);
	sg.setParent(2, 1, true);
	ASSERT_EQ(sg.root.children.size(), 1);
	ASSERT_EQ(sg.root.children[0]->children[0]->entity, 2);
}

TEST(SceneGraphTest, SceneGraphRemoveEntity)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	sg.addEntity(2);
	sg.setParent(2, 1, true);
	sg.removeEntity(1);
	ASSERT_EQ(sg.root.children.size(), 1);
	ASSERT_EQ(sg.root.children[0]->entity, 2);
}

#endif //ICE_SCENEGRAPHTEST_H
