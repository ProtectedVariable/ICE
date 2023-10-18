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
    ab.addResource<Material>("a_ice_test_mtl", new Resource(&mtl, {}));
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), &mtl);
    ASSERT_EQ(ab.getAsset<Material>("lol"), nullptr);

    auto dummy_vert = std::vector<Eigen::Vector3f>();
    dummy_vert.emplace_back(1,1,1);
    Mesh mesh = Mesh(dummy_vert, std::vector<Eigen::Vector3f>(), std::vector<Eigen::Vector2f>(),std::vector<Eigen::Vector3i>());
    ab.addResource<Mesh>("a_ice_test_mesh", new Resource(&mesh, {}));
    ASSERT_EQ(ab.getAsset<Mesh>("a_ice_test_mesh"), &mesh);
    ASSERT_EQ(ab.getAsset<Mesh>("lel"), nullptr);

    Texture* tex = Texture2D::Create("Not needed for this");
    ab.addResource<Texture2D>("a_ice_test_tex", new Resource(tex, {}));
    ASSERT_EQ(ab.getAsset<Texture2D>("a_ice_test_tex"), tex);
    ASSERT_EQ(ab.getAsset<Texture2D>("lil"), nullptr);

    Shader* shader = Shader::Create("","");
    ab.addResource<Shader>("a_ice_test_shader", new Resource(shader, {}));
    ASSERT_EQ(ab.getAsset<Shader>("a_ice_test_shader"), shader);
    ASSERT_EQ(ab.getAsset<Shader>("lul"), nullptr);
}

TEST(AssetBankTest, AssetsCanBeRenamed)
{
    RendererAPI::SetAPI(None);
    AssetBank ab = AssetBank();
    Material mtl = Material();
    ab.addResource<Material>("a_ice_test_mtl", new Resource(&mtl, {}));
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), &mtl);
    ASSERT_EQ(ab.getAsset<Material>("lol"), nullptr);

    ab.renameAsset(AssetPath("Material/a_ice_test_mtl"), AssetPath("Material/lol"));
    ASSERT_EQ(ab.getAsset<Material>("lol"), &mtl);
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), nullptr);
}

TEST(AssetBankTest, GetNameReturnsCorrectName)
{
    RendererAPI::SetAPI(None);
    AssetBank ab = AssetBank();
    Material mtl = Material();
    ab.addResource<Material>("a_ice_test_mtl", new Resource(&mtl, {}));
    ASSERT_EQ(AssetPath("Material/a_ice_test_mtl"), ab.getName(ab.getUID(AssetPath("Material/a_ice_test_mtl"))));
    ASSERT_EQ(AssetPath(""), ab.getName(0));
}

TEST(AssetBankTest, NameInUseBehavesCorrectly)
{
    RendererAPI::SetAPI(None);
    AssetBank ab = AssetBank();
    Material mtl = Material();
    ab.addResource<Material>("a_ice_test_mtl", new Resource(&mtl, {}));
    ASSERT_TRUE(ab.nameInUse(AssetPath::WithTypePrefix<Material>("a_ice_test_mtl")));
    ASSERT_FALSE(ab.nameInUse(AssetPath::WithTypePrefix<Material>("hey")));
}

#endif //ICE_ASSETBANKTEST_H
