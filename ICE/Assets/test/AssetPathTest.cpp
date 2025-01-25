//
// Created by Thomas Ibanez on 24.02.21.
//
#include <Material.h>
#include <Model.h>
#include <Shader.h>
#include <Texture.h>
#include <gtest/gtest.h>

#include "AssetPath.h"

using namespace ICE;

TEST(AssetPathTest, ToStringOk) {
    EXPECT_EQ(AssetPath("test").toString(), "test");
    EXPECT_EQ(AssetPath("test/test_1").toString(), "test/test_1");
}

TEST(AssetPathTest, PathOk) {
    EXPECT_EQ(AssetPath("test").prefix(), "");
    EXPECT_EQ(AssetPath("test/test_1").prefix(), "test/");
    EXPECT_EQ(AssetPath("test0/test_1/test_2").prefix(), "test0/test_1/");
    EXPECT_EQ(AssetPath("test0/test_1/test_2").getName(), "test_2");
}

TEST(AssetPathTest, setNameOk) {
    AssetPath a("test_0/test_1/test_2/test");
    a.setName("toast");
    EXPECT_EQ(a.getName(), "toast");
    EXPECT_EQ(a.toString(), "test_0/test_1/test_2/toast");
}

TEST(AssetPathTest, TypePrefixOk) {
    EXPECT_EQ(AssetPath::WithTypePrefix<Material>("a").toString(), "Materials/a");
    EXPECT_EQ(AssetPath::WithTypePrefix<Model>("a").toString(), "Models/a");
    EXPECT_EQ(AssetPath::WithTypePrefix<Shader>("a").toString(), "Shaders/a");
    EXPECT_EQ(AssetPath::WithTypePrefix<Texture2D>("a").toString(), "Textures/a");
    EXPECT_EQ(AssetPath::WithTypePrefix<TextureCube>("a").toString(), "CubeMaps/a");
}
