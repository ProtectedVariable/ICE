#include <gtest/gtest.h>

#include <AssetBank.h>
#include <AudioClip.h>
#include <AudioListenerComponent.h>
#include <AudioSourceComponent.h>
#include <DefaultLoaders.h>
#include <PerspectiveCamera.h>
#include <Project.h>
#include <TransformComponent.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>

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

// --- Audio persistence (phase 3) ----------------------------------------------------------------

namespace {
// Write a tiny mono WAV so the round-trip exercises the real AudioClipLoader rather than a stub:
// clips persist by source path, so load must be able to decode the file again.
void writeMonoWav(const fs::path &path, int frames = 64) {
    std::vector<int16_t> pcm(frames, 1234);
    const uint32_t data_bytes = static_cast<uint32_t>(pcm.size() * 2);
    std::ofstream out(path, std::ios::binary);
    auto u32 = [&](uint32_t v) { out.write(reinterpret_cast<const char *>(&v), 4); };
    auto u16 = [&](uint16_t v) { out.write(reinterpret_cast<const char *>(&v), 2); };
    out.write("RIFF", 4);
    u32(36 + data_bytes);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    u32(16);
    u16(1);  // PCM
    u16(1);  // mono
    u32(44100);
    u32(44100 * 2);
    u16(2);
    u16(16);
    out.write("data", 4);
    u32(data_bytes);
    out.write(reinterpret_cast<const char *>(pcm.data()), data_bytes);
}
}  // namespace

TEST(ProjectTest, AudioClipRoundTrips) {
    fs::path base = freshBase();
    auto cam = makeCamera();

    fs::path wav = base / "tone.wav";
    writeMonoWav(wav);

    AssetUID uid = NO_ASSET_ID;
    {
        Project proj(base, "Proj");
        fs::create_directories(proj.getBaseDirectory());
        uid = proj.importAudio("tone", wav);
        ASSERT_NE(uid, NO_ASSET_ID);
        proj.writeToFile(cam);
    }
    {
        Project proj(base, "Proj");
        proj.loadFromFile();
        auto clip = proj.getAssetBank()->getAsset<AudioClip>("tone");
        ASSERT_NE(clip, nullptr) << "the clip must be decoded again from its persisted source path";
        EXPECT_EQ(clip->getChannels(), 1u);
        EXPECT_EQ(clip->getSampleRate(), 44100u);
        EXPECT_EQ(proj.audioClip("tone"), uid) << "UIDs must be stable across save/load";
    }
    std::error_code ec;
    fs::remove_all(base, ec);
}

// A stereo source imported for 3D is folded to mono at import, and stays mono across a round-trip
// (the downmixed PCM is what gets written to the project's Audio folder).
TEST(ProjectTest, ThreeDAudioImportIsMono) {
    fs::path base = freshBase();
    fs::path wav = base / "stereo.wav";

    // Hand-built stereo WAV.
    {
        const int frames = 32;
        std::vector<int16_t> pcm(frames * 2, 500);
        const uint32_t data_bytes = static_cast<uint32_t>(pcm.size() * 2);
        std::ofstream out(wav, std::ios::binary);
        auto u32 = [&](uint32_t v) { out.write(reinterpret_cast<const char *>(&v), 4); };
        auto u16 = [&](uint16_t v) { out.write(reinterpret_cast<const char *>(&v), 2); };
        out.write("RIFF", 4);
        u32(36 + data_bytes);
        out.write("WAVE", 4);
        out.write("fmt ", 4);
        u32(16);
        u16(1);
        u16(2);
        u32(44100);
        u32(44100 * 4);
        u16(4);
        u16(16);
        out.write("data", 4);
        u32(data_bytes);
        out.write(reinterpret_cast<const char *>(pcm.data()), data_bytes);
    }

    Project proj(base, "Proj");
    fs::create_directories(proj.getBaseDirectory());

    AssetUID flat = proj.importAudio("music", wav, false);
    ASSERT_NE(flat, NO_ASSET_ID);
    EXPECT_EQ(proj.getAssetBank()->getAsset<AudioClip>(flat)->getChannels(), 2u) << "a 2D import keeps its stereo image";

    AssetUID spatial = proj.importAudio("footstep", wav, true);
    ASSERT_NE(spatial, NO_ASSET_ID);
    auto clip = proj.getAssetBank()->getAsset<AudioClip>(spatial);
    EXPECT_TRUE(clip->isMono()) << "a 3D import must be mono or OpenAL will not spatialize it";
    EXPECT_EQ(clip->getFrameCount(), 32u) << "downmix must preserve the frame count";

    std::error_code ec;
    fs::remove_all(base, ec);
}

TEST(ProjectTest, AudioComponentsRoundTrip) {
    fs::path base = freshBase();
    auto cam = makeCamera();
    fs::path wav = base / "tone.wav";
    writeMonoWav(wav);

    AssetUID clip_uid = NO_ASSET_ID;
    {
        Project proj(base, "Proj");
        fs::create_directories(proj.getBaseDirectory());
        clip_uid = proj.importAudio("tone", wav);

        Scene scene("MainScene");
        // createEntity() already registers the entity; calling addEntity() on it as well would
        // list it twice and duplicate its components on load.
        Entity e = scene.createEntity();
        scene.setAlias(e, "Emitter");
        scene.getRegistry()->addComponent(e, TransformComponent(Eigen::Vector3f(1, 2, 3)));

        AudioSourceComponent source(clip_uid);
        source.volume = 0.4f;
        source.pitch = 1.25f;
        source.loop = true;
        source.playOnAwake = true;
        source.spatial = true;
        source.minDistance = 2.5f;
        source.maxDistance = 60.0f;
        source.rolloff = 0.75f;
        source.priority = 200;
        source.bus = 1;
        // Runtime state that must NOT survive the round-trip.
        source.voice_index = 7;
        source.voice_generation = 9;
        source.state = AudioSourceState::Playing;
        scene.getRegistry()->addComponent(e, source);

        AudioListenerComponent listener;
        listener.volume = 0.8f;
        listener.active = false;
        scene.getRegistry()->addComponent(e, listener);

        proj.addScene(scene);
        proj.setCurrentScene(proj.getScenes()[0]);
        proj.writeToFile(cam);
    }
    {
        Project proj(base, "Proj");
        proj.loadFromFile();
        auto scene = proj.getCurrentScene();
        ASSERT_NE(scene, nullptr);

        Entity found = NULL_ENTITY;
        for (Entity e : scene->getRegistry()->getEntities()) {
            if (scene->getRegistry()->entityHasComponent<AudioSourceComponent>(e)) {
                found = e;
                break;
            }
        }
        ASSERT_NE(found, NULL_ENTITY) << "the AudioSourceComponent should have been persisted";

        auto *source = scene->getRegistry()->getComponent<AudioSourceComponent>(found);
        EXPECT_EQ(source->clip, clip_uid);
        EXPECT_FLOAT_EQ(source->volume, 0.4f);
        EXPECT_FLOAT_EQ(source->pitch, 1.25f);
        EXPECT_TRUE(source->loop);
        EXPECT_TRUE(source->playOnAwake);
        EXPECT_TRUE(source->spatial);
        EXPECT_FLOAT_EQ(source->minDistance, 2.5f);
        EXPECT_FLOAT_EQ(source->maxDistance, 60.0f);
        EXPECT_FLOAT_EQ(source->rolloff, 0.75f);
        EXPECT_EQ(source->priority, 200);
        EXPECT_EQ(source->bus, 1);

        // Runtime state must come back clean: a restored scene that pointed at a voice from the
        // previous run would be controlling whatever sound now occupies that slot.
        EXPECT_EQ(source->voice_index, 0u);
        EXPECT_EQ(source->voice_generation, 0u);
        EXPECT_FALSE(source->voice_started);
        EXPECT_FALSE(source->awake_handled);
        EXPECT_EQ(source->state, AudioSourceState::Stopped) << "a scene must load quiescent; playOnAwake starts it";

        ASSERT_TRUE(scene->getRegistry()->entityHasComponent<AudioListenerComponent>(found));
        auto *listener = scene->getRegistry()->getComponent<AudioListenerComponent>(found);
        EXPECT_FLOAT_EQ(listener->volume, 0.8f);
        EXPECT_FALSE(listener->active);
    }
    std::error_code ec;
    fs::remove_all(base, ec);
}

TEST(ProjectTest, MixerLevelsRoundTrip) {
    fs::path base = freshBase();
    auto cam = makeCamera();
    {
        Project proj(base, "Proj");
        fs::create_directories(proj.getBaseDirectory());
        proj.setBusGains({1.0f, 0.5f, 0.25f, 0.75f, 1.0f});
        proj.setBusMutes({false, true, false, false, false});
        proj.writeToFile(cam);
    }
    {
        Project proj(base, "Proj");
        proj.loadFromFile();
        ASSERT_EQ(proj.getBusGains().size(), 5u);
        EXPECT_FLOAT_EQ(proj.getBusGains()[1], 0.5f);
        EXPECT_FLOAT_EQ(proj.getBusGains()[2], 0.25f);
        ASSERT_EQ(proj.getBusMutes().size(), 5u);
        EXPECT_TRUE(proj.getBusMutes()[1]);
        EXPECT_FALSE(proj.getBusMutes()[0]);
    }
    std::error_code ec;
    fs::remove_all(base, ec);
}

// A project written before audio existed has no "audioClips"/"audioMixer" keys. Loading it must
// not throw -- older projects have to keep opening.
TEST(ProjectTest, ProjectWithoutAudioSectionsStillLoads) {
    fs::path base = freshBase();
    auto cam = makeCamera();
    {
        Project proj(base, "Proj");
        fs::create_directories(proj.getBaseDirectory());
        proj.writeToFile(cam);
    }
    // Strip the audio keys, mimicking a pre-audio project file.
    fs::path file = base / "Proj" / "Proj.ice";
    json j;
    {
        std::ifstream in(file);
        ASSERT_TRUE(in.is_open());
        in >> j;
    }
    j.erase("audioClips");
    j.erase("audioMixer");
    {
        std::ofstream out(file);
        out << j.dump(4);
    }
    {
        Project proj(base, "Proj");
        proj.loadFromFile();  // must not throw
        EXPECT_TRUE(proj.getBusGains().empty()) << "an unauthored mix stays empty, leaving engine defaults";
    }
    std::error_code ec;
    fs::remove_all(base, ec);
}
