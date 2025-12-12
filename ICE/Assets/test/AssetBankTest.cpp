//
// Created by Thomas Ibanez on 24.02.21.
//
#define STB_IMAGE_IMPLEMENTATION
#include <gtest/gtest.h>

#include "AssetBank.h"
#include "NoneGraphicsFactory.h"

using namespace ICE;


TEST(AssetBankTest, AddedAssetsCanBeRetrieved) {
    NoneGraphicsFactory g_fac;
    AssetBank ab(std::make_shared<NoneGraphicsFactory>(g_fac));
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), mtl);
    ASSERT_EQ(ab.getAsset<Material>("lol"), nullptr);

    auto mesh = std::make_shared<Mesh>(MeshData{});
    ab.addAsset<Mesh>("a_ice_test_mesh", mesh);
    ASSERT_EQ(ab.getAsset<Mesh>("a_ice_test_mesh"), mesh);
    ASSERT_EQ(ab.getAsset<Mesh>("lel"), nullptr);

    auto tex = g_fac.createTexture2D("Not needed for this");
    ab.addAsset<Texture2D>("a_ice_test_tex", tex);
    ASSERT_EQ(ab.getAsset<Texture2D>("a_ice_test_tex"), tex);
    ASSERT_EQ(ab.getAsset<Texture2D>("lil"), nullptr);

    auto shader = g_fac.createShader("", "");
    ab.addAsset<Shader>("a_ice_test_shader", shader);
    ASSERT_EQ(ab.getAsset<Shader>("a_ice_test_shader"), shader);
    ASSERT_EQ(ab.getAsset<Shader>("lul"), nullptr);
}

TEST(AssetBankTest, AssetsCanBeRenamed) {
    AssetBank ab(std::make_shared<NoneGraphicsFactory>());
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), mtl);
    ASSERT_EQ(ab.getAsset<Material>("lol"), nullptr);

    ab.renameAsset(AssetPath("Materials/a_ice_test_mtl"), AssetPath("Materials/lol"));
    ASSERT_EQ(ab.getAsset<Material>("lol"), mtl);
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), nullptr);
}

TEST(AssetBankTest, GetNameReturnsCorrectName) {
    AssetBank ab(std::make_shared<NoneGraphicsFactory>());
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_EQ(AssetPath("Materials/a_ice_test_mtl"), ab.getName(ab.getUID(AssetPath("Materials/a_ice_test_mtl"))));
    ASSERT_EQ(AssetPath(""), ab.getName(0));
}

TEST(AssetBankTest, NameInUseBehavesCorrectly) {
    AssetBank ab(std::make_shared<NoneGraphicsFactory>());
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_TRUE(ab.nameInUse(AssetPath::WithTypePrefix<Material>("a_ice_test_mtl")));
    ASSERT_FALSE(ab.nameInUse(AssetPath::WithTypePrefix<Material>("hey")));
}
