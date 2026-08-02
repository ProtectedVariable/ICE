#ifndef ICE_SCENEGRAPHTEST_H
#define ICE_SCENEGRAPHTEST_H
#include <gtest/gtest.h>
#include <Scene.h>

using namespace ICE;

TEST(SceneGraphTest, SceneGraphCreated)
{
	SceneGraph sg = SceneGraph();
	ASSERT_EQ(sg.getRoot()->entity, 0);
}

TEST(SceneGraphTest, SceneGraphAddEntity)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	ASSERT_EQ(sg.getRoot()->children[0]->entity, 1);
}

TEST(SceneGraphTest, SceneGraphSetParent)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	sg.addEntity(2);
	ASSERT_EQ(sg.getRoot()->children[1]->entity, 2);
	sg.setParent(2, 1);
	ASSERT_EQ(sg.getRoot()->children.size(), 1);
	ASSERT_EQ(sg.getRoot()->children[0]->children[0]->entity, 2);
}

TEST(SceneGraphTest, SceneGraphRemoveEntity)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	sg.addEntity(2);
	sg.setParent(2, 1);
	sg.removeEntity(1);
	ASSERT_EQ(sg.getRoot()->children.size(), 1);
	ASSERT_EQ(sg.getRoot()->children[0]->entity, 2);
}

TEST(SceneGraphTest, SceneGraphCollectSubtree)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	sg.addEntity(2);
	sg.addEntity(3);
	sg.setParent(2, 1);
	sg.setParent(3, 2);
	auto subtree = sg.collectSubtree(1);
	ASSERT_EQ(subtree.size(), 3);
	ASSERT_EQ(subtree[0], 1);
	ASSERT_TRUE(sg.collectSubtree(42).empty());
}

TEST(SceneGraphTest, SceneGraphRemoveSubtree)
{
	SceneGraph sg = SceneGraph();
	sg.addEntity(1);
	sg.addEntity(2);
	sg.addEntity(3);
	sg.setParent(2, 1);
	sg.setParent(3, 2);
	sg.removeSubtree(1);
	ASSERT_EQ(sg.getRoot()->children.size(), 0);
	ASSERT_TRUE(sg.collectSubtree(2).empty());
	ASSERT_TRUE(sg.collectSubtree(3).empty());
}

#endif //ICE_SCENEGRAPHTEST_H
