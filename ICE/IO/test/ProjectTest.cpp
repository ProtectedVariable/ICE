#include <gtest/gtest.h>

#include <AssetBank.h>
#include <PerspectiveCamera.h>
#include <Project.h>

#include <filesystem>
#include <memory>

using namespace ICE;
namespace fs = std::filesystem;

namespace {
// A plugin-style asset kind + loader used only by this suite. The loader ignores its sources and
// returns a fixed asset, so the round-trip does not depend on any on-disk source file.
class TestCustomAsset : public Asset {
   public:
    explicit TestCustomAsset(int v = 0) : value(v) {}
    AssetType getType() const override { return AssetType::EOther; }
    std::string getTypeName() const override { return "TestCustom"; }
    int value;
};

class TestCustomLoader : public IAssetLoader<TestCustomAsset> {
   public:
    std::shared_ptr<TestCustomAsset> load(const std::vector<fs::path> &) override { return std::make_shared<TestCustomAsset>(123); }
};

// A fresh, empty project base directory under the system temp dir.
fs::path freshBase() {
    static int counter = 0;
    fs::path base = fs::temp_directory_path() / ("ice_proj_test_" + std::to_string(++counter));
    std::error_code ec;
    fs::remove_all(base, ec);
    fs::create_directories(base);
    return base;
}

std::shared_ptr<PerspectiveCamera> makeCamera() { return std::make_shared<PerspectiveCamera>(70.0, 1.0, 0.1, 100.0); }
}  // namespace

// A custom (plugin-defined) asset survives save -> load through the generic "assets" section, keyed
// by its path prefix and reloaded via the erased loader.
TEST(ProjectTest, CustomAssetTypeRoundTrips) {
    fs::path base = freshBase();
    auto cam = makeCamera();

    AssetUID uid = NO_ASSET_ID;
    {
        Project proj(base, "Proj");
        fs::create_directories(proj.getBaseDirectory());
        proj.getAssetBank()->addLoader<TestCustomAsset>("TestCustom", std::make_shared<TestCustomLoader>());
        ASSERT_TRUE(proj.getAssetBank()->addAsset<TestCustomAsset>("thing", std::make_shared<TestCustomAsset>(7)));
        uid = proj.getAssetBank()->getUID(AssetPath::WithTypePrefix<TestCustomAsset>("thing"));
        proj.writeToFile(cam);
    }
    {
        Project proj(base, "Proj");
        proj.getAssetBank()->addLoader<TestCustomAsset>("TestCustom", std::make_shared<TestCustomLoader>());
        proj.loadFromFile();
        auto asset = proj.getAssetBank()->getAsset<TestCustomAsset>("thing");
        ASSERT_NE(asset, nullptr);
        ASSERT_EQ(asset->value, 123);  // came back through the loader, not by value
        ASSERT_EQ(proj.getAssetBank()->getUID(AssetPath::WithTypePrefix<TestCustomAsset>("thing")), uid);
    }
    std::error_code ec;
    fs::remove_all(base, ec);
}

// When the providing plugin/loader isn't available at load, the entry is not dropped: it is
// preserved and re-emitted on the next save, so re-loading with the loader present restores it.
TEST(ProjectTest, MissingLoaderPreservesCustomAssetAcrossSave) {
    fs::path base = freshBase();
    auto cam = makeCamera();

    {
        Project proj(base, "Proj");
        fs::create_directories(proj.getBaseDirectory());
        proj.getAssetBank()->addLoader<TestCustomAsset>("TestCustom", std::make_shared<TestCustomLoader>());
        proj.getAssetBank()->addAsset<TestCustomAsset>("thing", std::make_shared<TestCustomAsset>(7));
        proj.writeToFile(cam);
    }
    {
        // Load with no loader registered on this bank: the asset can't be built, so it is preserved
        // and written back out verbatim.
        Project proj(base, "Proj");
        proj.loadFromFile();
        ASSERT_EQ(proj.getAssetBank()->getUID(AssetPath::WithTypePrefix<TestCustomAsset>("thing")), NO_ASSET_ID);
        proj.writeToFile(cam);
    }
    {
        // With the loader present again, the preserved entry loads normally.
        Project proj(base, "Proj");
        proj.getAssetBank()->addLoader<TestCustomAsset>("TestCustom", std::make_shared<TestCustomLoader>());
        proj.loadFromFile();
        ASSERT_NE(proj.getAssetBank()->getAsset<TestCustomAsset>("thing"), nullptr);
    }
    std::error_code ec;
    fs::remove_all(base, ec);
}
