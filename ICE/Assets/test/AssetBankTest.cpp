//
// Created by Thomas Ibanez on 24.02.21.
//
#include <gtest/gtest.h>

#include <typeindex>

#include "AssetBank.h"
#include "AssetLoader.h"
#include "JobScheduler.h"

using namespace ICE;

namespace {
// A brand-new asset type unknown to the engine's built-in set -- proves the loader registry is open
// (type-erased) rather than closed to a fixed variant of concrete types.
class DummyAsset : public Asset {
   public:
    explicit DummyAsset(int v) : value(v) {}
    AssetType getType() const override { return AssetType::EOther; }
    std::string getTypeName() const override { return "Dummy"; }
    int value;
};

class DummyLoader : public IAssetLoader<DummyAsset> {
   public:
    std::shared_ptr<DummyAsset> load(const std::vector<std::filesystem::path>& files) override {
        return std::make_shared<DummyAsset>(static_cast<int>(files.size()));
    }
};

// Distinct dummy types (by template tag) for prefix-conflict tests, so each test uses types no other
// test registers -- the AssetPath registration maps are process-global static state.
template<int Tag>
class TaggedDummyAsset : public Asset {
   public:
    AssetType getType() const override { return AssetType::EOther; }
    std::string getTypeName() const override { return "TaggedDummy"; }
};
}  // namespace

TEST(AssetBankTest, AddedAssetsCanBeRetrieved) {
    AssetBank ab;
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), mtl);
    ASSERT_EQ(ab.getAsset<Material>("lol"), nullptr);

    auto mesh = std::make_shared<Mesh>(MeshData{{Eigen::Vector3f()}});
    ab.addAsset<Mesh>("a_ice_test_mesh", mesh);
    ASSERT_EQ(ab.getAsset<Mesh>("a_ice_test_mesh"), mesh);
    ASSERT_EQ(ab.getAsset<Mesh>("lel"), nullptr);

    auto tex = std::make_shared<Texture2D>(nullptr, 0, 0, ICE::TextureFormat::SRGB8);
    ab.addAsset<Texture2D>("a_ice_test_tex", tex);
    ASSERT_EQ(ab.getAsset<Texture2D>("a_ice_test_tex"), tex);
    ASSERT_EQ(ab.getAsset<Texture2D>("lil"), nullptr);

    auto shader = std::make_shared<Shader>();
    ab.addAsset<Shader>("a_ice_test_shader", shader);
    ASSERT_EQ(ab.getAsset<Shader>("a_ice_test_shader"), shader);
    ASSERT_EQ(ab.getAsset<Shader>("lul"), nullptr);
}

TEST(AssetBankTest, AssetsCanBeRenamed) {
    AssetBank ab;
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), mtl);
    ASSERT_EQ(ab.getAsset<Material>("lol"), nullptr);

    ab.renameAsset(AssetPath("Materials/a_ice_test_mtl"), AssetPath("Materials/lol"));
    ASSERT_EQ(ab.getAsset<Material>("lol"), mtl);
    ASSERT_EQ(ab.getAsset<Material>("a_ice_test_mtl"), nullptr);
}

TEST(AssetBankTest, GetNameReturnsCorrectName) {
    AssetBank ab;
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_EQ(AssetPath("Materials/a_ice_test_mtl"), ab.getName(ab.getUID(AssetPath("Materials/a_ice_test_mtl"))));
    ASSERT_EQ(AssetPath(""), ab.getName(0));
}

TEST(AssetBankTest, NameInUseBehavesCorrectly) {
    AssetBank ab;
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    ASSERT_TRUE(ab.nameInUse(AssetPath::WithTypePrefix<Material>("a_ice_test_mtl")));
    ASSERT_FALSE(ab.nameInUse(AssetPath::WithTypePrefix<Material>("hey")));
}

// Removing an asset must notify listeners with the removed UID -- this is the seam the GPU registry
// subscribes to so it can evict the matching GPU upload (otherwise removed/re-imported assets leak
// and keep rendering stale data).
TEST(AssetBankTest, RemovalListenerFiresWithRemovedUID) {
    AssetBank ab;
    auto mtl = std::make_shared<Material>();
    ab.addAsset<Material>("a_ice_test_mtl", mtl);
    AssetUID uid = ab.getUID(AssetPath::WithTypePrefix<Material>("a_ice_test_mtl"));

    std::vector<AssetUID> removed;
    ab.addRemovalListener([&](AssetUID id) { removed.push_back(id); });

    ASSERT_TRUE(ab.removeAsset(AssetPath::WithTypePrefix<Material>("a_ice_test_mtl")));
    ASSERT_EQ(removed.size(), 1u);
    ASSERT_EQ(removed[0], uid);

    // Removing a non-existent asset does not fire the listener.
    ASSERT_FALSE(ab.removeAsset(AssetPath::WithTypePrefix<Material>("a_ice_test_mtl")));
    ASSERT_EQ(removed.size(), 1u);
}

// An unregistered listener must not be invoked (the GPU registry unregisters in its destructor to
// avoid a dangling capture of `this`).
TEST(AssetBankTest, RemovalListenerCanBeUnregistered) {
    AssetBank ab;
    ab.addAsset<Material>("a_ice_test_mtl", std::make_shared<Material>());

    int calls = 0;
    auto handle = ab.addRemovalListener([&](AssetUID) { ++calls; });
    ab.removeRemovalListener(handle);

    ASSERT_TRUE(ab.removeAsset(AssetPath::WithTypePrefix<Material>("a_ice_test_mtl")));
    ASSERT_EQ(calls, 0);
}

// Re-import path at the bank level: removing then re-adding under the same name fires the listener
// (so the GPU registry drops the old upload) and getAsset returns the fresh asset.
TEST(AssetBankTest, ReAddUnderSameNameReplacesAssetAndNotifies) {
    AssetBank ab;
    auto first = std::make_shared<Mesh>(MeshData{{Eigen::Vector3f()}});
    ab.addAsset<Mesh>("a_ice_test_mesh", first);

    int notifications = 0;
    ab.addRemovalListener([&](AssetUID) { ++notifications; });

    ASSERT_TRUE(ab.removeAsset(AssetPath::WithTypePrefix<Mesh>("a_ice_test_mesh")));
    ASSERT_EQ(notifications, 1);

    auto second = std::make_shared<Mesh>(MeshData{{Eigen::Vector3f(), Eigen::Vector3f()}});
    ab.addAsset<Mesh>("a_ice_test_mesh", second);
    ASSERT_EQ(ab.getAsset<Mesh>("a_ice_test_mesh"), second);
    ASSERT_NE(ab.getAsset<Mesh>("a_ice_test_mesh"), first);
}

// The type-erased loader registry accepts an arbitrary Asset subclass and loads through both the
// typed and the type_index-keyed entry points.
TEST(AssetLoaderTest, RegistersAndLoadsCustomAssetType) {
    AssetLoader loader;
    loader.AddLoader<DummyAsset>(std::make_shared<DummyLoader>());

    auto typed = loader.LoadResource<DummyAsset>({"a", "b"});
    ASSERT_NE(typed, nullptr);
    ASSERT_EQ(typed->value, 2);

    // Erased path returns the same concrete type as an Asset.
    auto erased = loader.LoadResource(std::type_index(typeid(DummyAsset)), {"a"});
    ASSERT_NE(erased, nullptr);
    auto down = std::dynamic_pointer_cast<DummyAsset>(erased);
    ASSERT_NE(down, nullptr);
    ASSERT_EQ(down->value, 1);
}

// An unregistered type still throws (behavior preserved from the old variant-based registry).
TEST(AssetLoaderTest, ThrowsForUnregisteredType) {
    AssetLoader loader;
    ASSERT_THROW(loader.LoadResource<DummyAsset>({"x"}), ICEException);
}

// A plugin-defined asset kind works end-to-end once its loader + prefix are registered in one call:
// add / get / getAll / remove (with the eviction listener firing) all behave like a built-in type.
TEST(AssetBankTest, CustomAssetTypeEndToEnd) {
    AssetBank ab;
    ab.addLoader<DummyAsset>("DummyAssets", std::make_shared<DummyLoader>());

    auto d = std::make_shared<DummyAsset>(7);
    ASSERT_TRUE(ab.addAsset<DummyAsset>("thing", d));
    ASSERT_EQ(ab.getAsset<DummyAsset>("thing"), d);
    ASSERT_EQ(ab.getAll<DummyAsset>().size(), 1u);

    std::vector<AssetUID> removed;
    ab.addRemovalListener([&](AssetUID id) { removed.push_back(id); });
    AssetUID uid = ab.getUID(AssetPath::WithTypePrefix<DummyAsset>("thing"));

    ASSERT_TRUE(ab.removeAsset(AssetPath::WithTypePrefix<DummyAsset>("thing")));
    ASSERT_EQ(removed.size(), 1u);
    ASSERT_EQ(removed[0], uid);
    ASSERT_EQ(ab.getAsset<DummyAsset>("thing"), nullptr);
}

// Loading through a custom type's prefix routes to the right loader (reverse lookup for project
// persistence), and an unregistered prefix reports nothing.
TEST(AssetBankTest, PrefixReverseLookup) {
    AssetPath::registerType<TaggedDummyAsset<10>>("TaggedTen");
    auto found = AssetPath::typeForPrefix("TaggedTen");
    ASSERT_TRUE(found.has_value());
    ASSERT_EQ(found.value(), std::type_index(typeid(TaggedDummyAsset<10>)));
    ASSERT_FALSE(AssetPath::typeForPrefix("NoSuchPrefixEverRegistered").has_value());
}

// Duplicate/conflicting registration is rejected loudly; an identical re-registration is a no-op.
TEST(AssetBankTest, DuplicateTypeRegistrationRejected) {
    AssetPath::registerType<TaggedDummyAsset<1>>("ConflictPrefixX");

    // Same prefix, different type -> conflict.
    ASSERT_THROW(AssetPath::registerType<TaggedDummyAsset<2>>("ConflictPrefixX"), ICEException);
    // Same type, different prefix -> conflict.
    ASSERT_THROW(AssetPath::registerType<TaggedDummyAsset<1>>("DifferentPrefixY"), ICEException);
    // Identical (type, prefix) -> idempotent.
    ASSERT_NO_THROW(AssetPath::registerType<TaggedDummyAsset<1>>("ConflictPrefixX"));
}

// --- Async import (T5) ---------------------------------------------------------------------------

// Without a scheduler, requestAsset stages inline but still only publishes on pump(): the UID is
// reserved (Loading) immediately and becomes Ready with a payload after pump().
TEST(AssetBankTest, RequestAssetInlineBecomesReadyAfterPump) {
    AssetBank ab;
    ab.addLoader<DummyAsset>("DummyAssets", std::make_shared<DummyLoader>());

    AssetUID uid = ab.requestAsset<DummyAsset>("thing", std::vector<std::filesystem::path>{"a", "b"});
    // Reserved but not yet finalized.
    ASSERT_EQ(ab.getState(uid), AssetState::Loading);
    ASSERT_EQ(ab.getAsset<DummyAsset>(uid), nullptr);
    ASSERT_EQ(ab.inFlight(), 1u);

    ab.pump();
    ASSERT_EQ(ab.getState(uid), AssetState::Ready);
    ASSERT_EQ(ab.inFlight(), 0u);
    auto asset = ab.getAsset<DummyAsset>(uid);
    ASSERT_NE(asset, nullptr);
    ASSERT_EQ(asset->value, 2);  // DummyLoader returns files.size()
}

// With a scheduler, staging happens on a worker; flushAsync() drains and finalizes everything.
TEST(AssetBankTest, RequestAssetAsyncWithSchedulerFlushes) {
    AssetBank ab;
    ab.addLoader<DummyAsset>("DummyAssets", std::make_shared<DummyLoader>());
    auto scheduler = std::make_shared<JobScheduler>();
    ab.setScheduler(scheduler);

    std::vector<AssetUID> uids;
    for (int i = 0; i < 50; ++i) {
        uids.push_back(ab.requestAsset<DummyAsset>("thing" + std::to_string(i), std::vector<std::filesystem::path>{"a"}));
    }
    ab.flushAsync();
    ASSERT_EQ(ab.inFlight(), 0u);
    for (auto uid : uids) {
        ASSERT_EQ(ab.getState(uid), AssetState::Ready);
        ASSERT_NE(ab.getAsset<DummyAsset>(uid), nullptr);
    }
}

// A staging failure (null/throwing stage) leaves the entry Failed with a null payload -- no crash,
// no garbage asset.
TEST(AssetBankTest, RequestAssetFailedLoadMarksFailed) {
    AssetBank ab;
    AssetBank::StageFn stage = []() -> std::shared_ptr<void> { return nullptr; };
    AssetBank::CommitFn commit = [](const std::shared_ptr<void>& s) { return std::static_pointer_cast<Asset>(s); };
    AssetUID uid = ab.requestAsset<DummyAsset>("bad", stage, commit);

    ab.pump();
    ASSERT_EQ(ab.getState(uid), AssetState::Failed);
    ASSERT_EQ(ab.getAsset<DummyAsset>(uid), nullptr);
    ASSERT_EQ(ab.inFlight(), 0u);
}

// Requesting the same name twice returns the same reserved UID (no double-load).
TEST(AssetBankTest, RequestAssetDeDupesByName) {
    AssetBank ab;
    ab.addLoader<DummyAsset>("DummyAssets", std::make_shared<DummyLoader>());
    AssetUID a = ab.requestAsset<DummyAsset>("thing", std::vector<std::filesystem::path>{"a"});
    AssetUID b = ab.requestAsset<DummyAsset>("thing", std::vector<std::filesystem::path>{"a"});
    ASSERT_EQ(a, b);
    ASSERT_EQ(ab.inFlight(), 1u);
}

// The two-phase overload runs commit on the main thread in pump(), where it may add sub-assets --
// exactly how model import commits its meshes/materials/textures.
TEST(AssetBankTest, RequestAssetTwoPhaseCommitAddsSubAssets) {
    AssetBank ab;
    ab.addLoader<DummyAsset>("DummyAssets", std::make_shared<DummyLoader>());

    AssetBank::StageFn stage = []() -> std::shared_ptr<void> { return std::make_shared<int>(42); };
    AssetBank::CommitFn commit = [&ab](const std::shared_ptr<void>& staged) -> std::shared_ptr<Asset> {
        int payload = *std::static_pointer_cast<int>(staged);
        // Simulate a model committing a sub-asset, then producing the top-level asset.
        ab.addAsset<DummyAsset>("sub", std::make_shared<DummyAsset>(payload));
        return std::static_pointer_cast<Asset>(std::make_shared<DummyAsset>(7));
    };
    AssetUID uid = ab.requestAsset<DummyAsset>("top", stage, commit);
    ab.pump();

    ASSERT_EQ(ab.getState(uid), AssetState::Ready);
    ASSERT_EQ(ab.getAsset<DummyAsset>(uid)->value, 7);
    auto sub = ab.getAsset<DummyAsset>("sub");
    ASSERT_NE(sub, nullptr);
    ASSERT_EQ(sub->value, 42);
}

// The erased addAssetWithSpecificUID (keyed by std::type_index) restores a custom asset under a
// specific UID -- the primitive project loading uses to reload a persisted plugin asset by prefix.
TEST(AssetBankTest, AddAssetWithSpecificUIDErasedLoadsCustomType) {
    AssetBank ab;
    ab.addLoader<DummyAsset>("DummyAssets", std::make_shared<DummyLoader>());

    AssetPath path = AssetPath::WithTypePrefix<DummyAsset>("restored");
    std::type_index type = typeid(DummyAsset);
    ASSERT_TRUE(ab.addAssetWithSpecificUID(type, path, {"a", "b", "c"}, 42));

    ASSERT_EQ(ab.getUID(path), 42u);
    auto asset = ab.getAsset<DummyAsset>(42);
    ASSERT_NE(asset, nullptr);
    ASSERT_EQ(asset->value, 3);  // DummyLoader returns files.size()

    // Re-inserting the same UID/name is rejected.
    ASSERT_FALSE(ab.addAssetWithSpecificUID(type, path, {"x"}, 42));
}
