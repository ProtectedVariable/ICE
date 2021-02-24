//
// Created by Thomas Ibanez on 24.02.21.
//

#ifndef ICE_ASSETBANKTEST_H
#define ICE_ASSETBANKTEST_H
#include <gtest/gtest.h>

TEST(AssetBankTest, AddedAssetsCanBeRetrieved)
{
    RendererAPI::SetAPI(None);
    AssetBank ab = AssetBank();
    Material mtl = Material();
    ab.addMaterial("a_ice_test_mtl", &mtl);
    ASSERT_EQ(ab.getMaterial("a_ice_test_mtl"), &mtl);
    ASSERT_EQ(ab.getMaterial("lol"), nullptr);

    auto dummy_vert = std::vector<Eigen::Vector3f>();
    dummy_vert.emplace_back(1,1,1);
    Mesh mesh = Mesh(dummy_vert, std::vector<Eigen::Vector3f>(), std::vector<Eigen::Vector2f>(),std::vector<Eigen::Vector3i>());
    ab.addMesh("a_ice_test_mesh", &mesh);
    ASSERT_EQ(ab.getMesh("a_ice_test_mesh"), &mesh);
    ASSERT_EQ(ab.getMesh("lel"), nullptr);

    Texture* tex = Texture2D::Create("Not needed for this");
    ab.addTexture("a_ice_test_tex", tex);
    ASSERT_EQ(ab.getTexture("a_ice_test_tex"), tex);
    ASSERT_EQ(ab.getTexture("lil"), nullptr);

    Shader* shader = Shader::Create("","");
    ab.addShader("a_ice_test_shader", shader);
    ASSERT_EQ(ab.getShader("a_ice_test_shader"), shader);
    ASSERT_EQ(ab.getShader("lul"), nullptr);
}

TEST(AssetBankTest, AssetsCanBeRenamed)
{
    RendererAPI::SetAPI(None);
    AssetBank ab = AssetBank();
    Material mtl = Material();
    ab.addMaterial("a_ice_test_mtl", &mtl);
    ASSERT_EQ(ab.getMaterial("a_ice_test_mtl"), &mtl);
    ASSERT_EQ(ab.getMaterial("lol"), nullptr);

    ab.renameAsset("a_ice_test_mtl", "lol");
    ASSERT_EQ(ab.getMaterial("lol"), &mtl);
    ASSERT_EQ(ab.getMaterial("a_ice_test_mtl"), nullptr);
}

TEST(AssetBankTest, GetNameReturnsCorrectName)
{
    RendererAPI::SetAPI(None);
    AssetBank ab = AssetBank();
    Material mtl = Material();
    ab.addMaterial("a_ice_test_mtl", &mtl);
    ASSERT_EQ("a_ice_test_mtl", ab.getName(&mtl));
    ASSERT_EQ("null", ab.getName(nullptr));
}

TEST(AssetBankTest, NameInUseBehavesCorrectly)
{
    RendererAPI::SetAPI(None);
    AssetBank ab = AssetBank();
    Material mtl = Material();
    ab.addMaterial("a_ice_test_mtl", &mtl);
    ASSERT_TRUE(ab.nameInUse("a_ice_test_mtl"));
    ASSERT_FALSE(ab.nameInUse("hey"));
}

#endif //ICE_ASSETBANKTEST_H
