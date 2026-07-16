#include <AssetBank.h>
#include <gtest/gtest.h>

#include "MeshLoader.h"
#include "ModelLoader.h"

using namespace ICE;

TEST(ModelLoaderTest, LoadFromObj) {
    auto mesh = MeshLoader().load({"cube.obj"});
    EXPECT_EQ(mesh->getVertices().size(), 36);
    EXPECT_EQ(mesh->getIndices().size(), 12);
}

// stage() is pure: it must not mutate the AssetBank. All bank writes happen in commit(), which is
// what lets stage() run off the main thread.
TEST(ModelLoaderTest, StageDoesNotTouchBankCommitDoes) {
    AssetBank bank;
    ModelLoader loader(bank);

    StagedModel staged = loader.stage({"cube.obj"}, NO_ASSET_ID);
    ASSERT_TRUE(staged.valid);
    ASSERT_FALSE(staged.meshes.empty());
    ASSERT_EQ(bank.getAllEntries().size(), 0u) << "stage() must not mutate the bank";

    auto model = loader.commit(staged, bank);
    ASSERT_NE(model, nullptr);
    ASSERT_FALSE(model->getMeshes().empty());
    ASSERT_GT(bank.getAllEntries().size(), 0u) << "commit() populates the bank";
}

// Re-importing the same model preserves sub-asset UIDs (so existing scene/component references stay
// valid) and does not grow the bank.
TEST(ModelLoaderTest, ReimportPreservesUIDs) {
    AssetBank bank;
    ModelLoader loader(bank);

    auto first = loader.load({"cube.obj"});
    ASSERT_NE(first, nullptr);
    auto first_meshes = first->getMeshes();
    auto count_after_first = bank.getAllEntries().size();

    auto second = loader.load({"cube.obj"});
    ASSERT_NE(second, nullptr);
    ASSERT_EQ(second->getMeshes(), first_meshes) << "mesh UIDs changed on re-import";
    ASSERT_EQ(bank.getAllEntries().size(), count_after_first) << "re-import must not grow the bank";
}

// A failed parse (empty file list) stages nothing and commits nothing -- no partial bank state.
TEST(ModelLoaderTest, FailedParseCommitsNothing) {
    AssetBank bank;
    ModelLoader loader(bank);

    StagedModel staged = loader.stage({}, NO_ASSET_ID);
    ASSERT_FALSE(staged.valid);
    ASSERT_EQ(loader.commit(staged, bank), nullptr);
    ASSERT_EQ(bank.getAllEntries().size(), 0u);
}