#include <gtest/gtest.h>

#include <AudioClip.h>
#include <AudioEngine.h>
#include <NullAudioBackend.h>

#include <memory>

#include "MockAudioBackend.h"

using namespace ICE;

namespace {

// A bank holding one mono and one stereo clip, so tests can exercise both the spatial and the
// "this clip cannot be spatialized" paths.
struct Fixture {
    std::shared_ptr<AssetBank> bank = std::make_shared<AssetBank>();
    std::shared_ptr<MockAudioBackend> backend;
    std::unique_ptr<AudioEngine> audio;
    AssetUID mono = NO_ASSET_ID;
    AssetUID stereo = NO_ASSET_ID;

    explicit Fixture(std::size_t capacity = 4) : backend(std::make_shared<MockAudioBackend>(capacity)) {
        backend->initialize({});
        // 100 frames of silence is enough: nothing here inspects sample values.
        bank->addAsset<AudioClip>("mono", std::make_shared<AudioClip>(std::vector<int16_t>(100, 0), 1, 44100));
        bank->addAsset<AudioClip>("stereo", std::make_shared<AudioClip>(std::vector<int16_t>(200, 0), 2, 44100));
        mono = bank->getUID(AssetPath::WithTypePrefix<AudioClip>("mono"));
        stereo = bank->getUID(AssetPath::WithTypePrefix<AudioClip>("stereo"));
        audio = std::make_unique<AudioEngine>(backend, bank);
    }
};

}  // namespace

TEST(AudioEngineTest, PlayingAnUnknownClipIsSilentNotFatal) {
    Fixture f;
    EXPECT_FALSE(f.audio->play(NO_ASSET_ID).valid());
    EXPECT_FALSE(f.audio->play(123456).valid());
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);
}

TEST(AudioEngineTest, PlayStartsAVoiceAndUploadsTheClipOnce) {
    Fixture f;
    auto a = f.audio->play(f.mono);
    auto b = f.audio->play(f.mono);
    ASSERT_TRUE(a.valid());
    ASSERT_TRUE(b.valid());
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 2u);
    // Both voices share one uploaded buffer -- the registry caches by UID.
    EXPECT_EQ(f.backend->uploadCount, 1);
}

TEST(AudioEngineTest, TwoDPlaybackIsNotSpatialized) {
    Fixture f;
    auto v = f.audio->play(f.mono);
    ASSERT_NE(f.backend->voice(v), nullptr);
    EXPECT_FALSE(f.backend->voice(v)->params.spatial);
}

TEST(AudioEngineTest, PlayAtMarksTheVoiceSpatialAndCarriesPosition) {
    Fixture f;
    auto v = f.audio->playAt(f.mono, {1.0f, 2.0f, 3.0f});
    ASSERT_NE(f.backend->voice(v), nullptr);
    EXPECT_TRUE(f.backend->voice(v)->params.spatial);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.position.y(), 2.0f);
}

// OpenAL will not spatialize a stereo buffer -- it plays flat at full volume. Silently accepting
// that looks exactly like a broken 3D positioning bug, so the engine demotes it explicitly.
TEST(AudioEngineTest, StereoClipRequestedSpatiallyIsDemotedTo2D) {
    Fixture f;
    auto v = f.audio->playAt(f.stereo, {10.0f, 0.0f, 0.0f});
    ASSERT_TRUE(v.valid());
    ASSERT_NE(f.backend->voice(v), nullptr);
    EXPECT_FALSE(f.backend->voice(v)->params.spatial) << "a stereo clip must not be sent to the device as spatial";
}

TEST(AudioEngineTest, StopReleasesTheVoiceAndTheHandleGoesStale) {
    Fixture f;
    auto v = f.audio->play(f.mono);
    f.audio->stop(v);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);
    EXPECT_FALSE(f.audio->isPlaying(v));
    EXPECT_EQ(f.backend->voice(v), nullptr) << "handle must not resolve after release";
    f.audio->stop(v);  // double-stop must be harmless
}

TEST(AudioEngineTest, UpdateReclaimsFinishedOneShots) {
    Fixture f;
    auto v = f.audio->play(f.mono);
    f.backend->voice(v)->state = PlaybackState::Playing;
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 1u);

    f.backend->finish(v);
    f.audio->update(0.016);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);
    EXPECT_EQ(f.backend->updateCount, 1);
}

TEST(AudioEngineTest, LoopingVoicesAreNotReclaimed) {
    Fixture f;
    auto v = f.audio->play(f.mono, {.loop = true});
    for (int i = 0; i < 10; ++i) {
        f.audio->update(0.016);
    }
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 1u);
}

// --- Voice pool exhaustion and stealing ---------------------------------------------------------

TEST(AudioEngineTest, LowerPriorityVoiceIsStolenWhenThePoolIsFull) {
    Fixture f(2);
    auto quiet = f.audio->play(f.mono, {.priority = 10});
    auto loud = f.audio->play(f.mono, {.priority = 200});
    ASSERT_TRUE(quiet.valid());
    ASSERT_TRUE(loud.valid());

    auto important = f.audio->play(f.mono, {.priority = 250});
    ASSERT_TRUE(important.valid()) << "a high-priority sound must displace a low-priority one";
    EXPECT_EQ(f.audio->getStolenVoiceCount(), 1u);
    EXPECT_FALSE(f.audio->isPlaying(quiet)) << "the lowest-priority voice should be the victim";
    EXPECT_TRUE(f.audio->isPlaying(loud));
}

TEST(AudioEngineTest, AMoreImportantSoundIsNeverKilledForALessImportantOne) {
    Fixture f(2);
    f.audio->play(f.mono, {.priority = 200});
    f.audio->play(f.mono, {.priority = 200});

    auto trivial = f.audio->play(f.mono, {.priority = 5});
    EXPECT_FALSE(trivial.valid()) << "the incoming sound should be dropped, not steal a better one";
    EXPECT_EQ(f.audio->getStolenVoiceCount(), 0u);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 2u);
}

TEST(AudioEngineTest, EqualPriorityTieIsBrokenByAudibility) {
    Fixture f(2);
    f.audio->setListener({});  // listener at the origin
    // Same priority; the distant one is quieter at the listener and should lose.
    auto near = f.audio->playAt(f.mono, {1.0f, 0.0f, 0.0f}, {.priority = 100});
    auto far = f.audio->playAt(f.mono, {900.0f, 0.0f, 0.0f}, {.priority = 100});

    auto incoming = f.audio->playAt(f.mono, {2.0f, 0.0f, 0.0f}, {.priority = 100});
    ASSERT_TRUE(incoming.valid());
    EXPECT_TRUE(f.audio->isPlaying(near)) << "the closer, louder voice should survive";
    EXPECT_FALSE(f.audio->isPlaying(far));
}

// --- Gain, mute, listener -----------------------------------------------------------------------

TEST(AudioEngineTest, MasterGainScalesVoiceGainWithoutCompounding) {
    Fixture f;
    auto v = f.audio->play(f.mono, {.volume = 0.5f});
    f.audio->setMasterGain(0.5f);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.25f);

    // Re-applying must recompute from the stored per-voice gain, not multiply again.
    f.audio->setMasterGain(0.5f);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.25f);

    f.audio->setMasterGain(1.0f);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.5f);
}

TEST(AudioEngineTest, MuteSilencesAndUnmuteRestores) {
    Fixture f;
    auto v = f.audio->play(f.mono, {.volume = 0.8f});
    f.audio->setMuted(true);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.0f);
    f.audio->setMuted(false);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.8f);
}

TEST(AudioEngineTest, SetListenerReachesTheBackend) {
    Fixture f;
    ListenerState listener;
    listener.position = {5.0f, 0.0f, 0.0f};
    f.audio->setListener(listener);
    EXPECT_EQ(f.backend->setListenerCount, 1);
    EXPECT_FLOAT_EQ(f.backend->lastListener.position.x(), 5.0f);
}

TEST(AudioEngineTest, StaleHandleOperationsAreNoOps) {
    Fixture f;
    auto v = f.audio->play(f.mono);
    f.audio->stop(v);
    // Every one of these must tolerate a handle whose voice is long gone.
    f.audio->pause(v);
    f.audio->resume(v);
    f.audio->setVoiceParams(v, VoiceParams{});
    EXPECT_EQ(f.audio->getVoiceParams(v), nullptr);
    EXPECT_FALSE(f.audio->isPlaying(v));
}

// --- Registry / eviction ------------------------------------------------------------------------

TEST(AudioRegistryTest, RemovingAnAssetEvictsItsBuffer) {
    Fixture f;
    f.audio->play(f.mono);
    EXPECT_EQ(f.backend->residentBuffers(), 1u);

    f.bank->removeAsset(AssetPath::WithTypePrefix<AudioClip>("mono"));
    EXPECT_EQ(f.backend->releaseBufferCount, 1) << "the AssetBank removal listener should evict the upload";
    EXPECT_EQ(f.backend->residentBuffers(), 0u);
}

TEST(AudioRegistryTest, ReimportUploadsAFreshBuffer) {
    Fixture f;
    f.audio->play(f.mono);
    f.bank->removeAsset(AssetPath::WithTypePrefix<AudioClip>("mono"));

    f.bank->addAsset<AudioClip>("mono", std::make_shared<AudioClip>(std::vector<int16_t>(50, 0), 1, 22050));
    AssetUID reimported = f.bank->getUID(AssetPath::WithTypePrefix<AudioClip>("mono"));
    ASSERT_TRUE(f.audio->play(reimported).valid());
    EXPECT_EQ(f.backend->uploadCount, 2);
}

// --- Null backend -------------------------------------------------------------------------------

TEST(NullAudioBackendTest, EngineRunsFullyOnASilentBackend) {
    auto bank = std::make_shared<AssetBank>();
    bank->addAsset<AudioClip>("mono", std::make_shared<AudioClip>(std::vector<int16_t>(100, 0), 1, 44100));
    AssetUID clip = bank->getUID(AssetPath::WithTypePrefix<AudioClip>("mono"));

    auto backend = std::make_shared<NullAudioBackend>(2);
    backend->initialize({});
    AudioEngine audio(backend, bank);

    // This is the headless-CI path: everything must work, just inaudibly.
    auto v = audio.play(clip);
    EXPECT_TRUE(v.valid());
    EXPECT_TRUE(audio.isPlaying(v));
    audio.update(0.016);
    EXPECT_EQ(audio.getActiveVoiceCount(), 1u);

    audio.setMasterGain(0.5f);
    audio.setListener({});
    audio.stop(v);
    EXPECT_EQ(audio.getActiveVoiceCount(), 0u);
}

TEST(NullAudioBackendTest, RespectsItsCapacityAndStealingStillApplies) {
    auto bank = std::make_shared<AssetBank>();
    bank->addAsset<AudioClip>("c", std::make_shared<AudioClip>(std::vector<int16_t>(10, 0), 1, 8000));
    AssetUID clip = bank->getUID(AssetPath::WithTypePrefix<AudioClip>("c"));

    auto backend = std::make_shared<NullAudioBackend>(1);
    backend->initialize({});
    AudioEngine audio(backend, bank);

    ASSERT_TRUE(audio.play(clip, {.priority = 10}).valid());
    EXPECT_TRUE(audio.play(clip, {.priority = 250}).valid());
    EXPECT_EQ(audio.getStolenVoiceCount(), 1u);
    EXPECT_EQ(audio.getActiveVoiceCount(), 1u);
}

// --- AudioClip ----------------------------------------------------------------------------------

TEST(AudioClipTest, DerivesFrameCountAndDurationFromTheSampleBuffer) {
    AudioClip stereo(std::vector<int16_t>(200, 0), 2, 100);
    EXPECT_EQ(stereo.getFrameCount(), 100u);
    EXPECT_DOUBLE_EQ(stereo.getDuration(), 1.0);
    EXPECT_FALSE(stereo.isMono());

    AudioClip mono(std::vector<int16_t>(100, 0), 1, 100);
    EXPECT_EQ(mono.getFrameCount(), 100u);
    EXPECT_TRUE(mono.isMono());
}

TEST(AudioClipTest, DefaultConstructedClipIsEmptyAndDivisionSafe) {
    AudioClip clip;
    EXPECT_TRUE(clip.isEmpty());
    EXPECT_EQ(clip.getFrameCount(), 0u);   // must not divide by zero channels
    EXPECT_DOUBLE_EQ(clip.getDuration(), 0.0);  // must not divide by a zero sample rate
}

// --- Mixer buses (phase 3) ----------------------------------------------------------------------

TEST(AudioBusTest, BusGainScalesOnlyItsOwnVoices) {
    Fixture f;
    auto music = f.audio->play(f.mono, {.volume = 1.0f, .bus = BusId::Music});
    auto sfx = f.audio->play(f.mono, {.volume = 1.0f, .bus = BusId::SFX});

    f.audio->setBusGain(BusId::Music, 0.25f);

    EXPECT_FLOAT_EQ(f.backend->voice(music)->params.gain, 0.25f);
    EXPECT_FLOAT_EQ(f.backend->voice(sfx)->params.gain, 1.0f) << "another bus must be untouched";
}

TEST(AudioBusTest, BusMasterAndVoiceGainsMultiply) {
    Fixture f;
    auto v = f.audio->play(f.mono, {.volume = 0.5f, .bus = BusId::UI});
    f.audio->setBusGain(BusId::UI, 0.5f);
    f.audio->setMasterGain(0.5f);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.125f);
}

TEST(AudioBusTest, RepeatedBusChangesDoNotCompound) {
    Fixture f;
    auto v = f.audio->play(f.mono, {.volume = 1.0f, .bus = BusId::SFX});
    for (int i = 0; i < 5; ++i) {
        f.audio->setBusGain(BusId::SFX, 0.5f);
    }
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.5f) << "per-voice gain is the source of truth";
}

TEST(AudioBusTest, MutingABusSilencesItAndUnmuteRestores) {
    Fixture f;
    auto music = f.audio->play(f.mono, {.volume = 0.8f, .bus = BusId::Music});
    auto sfx = f.audio->play(f.mono, {.volume = 0.8f, .bus = BusId::SFX});

    f.audio->setBusMuted(BusId::Music, true);
    EXPECT_FLOAT_EQ(f.backend->voice(music)->params.gain, 0.0f);
    EXPECT_FLOAT_EQ(f.backend->voice(sfx)->params.gain, 0.8f);

    f.audio->setBusMuted(BusId::Music, false);
    EXPECT_FLOAT_EQ(f.backend->voice(music)->params.gain, 0.8f);
}

TEST(AudioBusTest, BusGainAppliesToVoicesStartedAfterTheChange) {
    Fixture f;
    f.audio->setBusGain(BusId::Voice, 0.5f);
    auto v = f.audio->play(f.mono, {.volume = 1.0f, .bus = BusId::Voice});
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.5f);
}

TEST(AudioBusTest, DefaultsAreUnityAndUnmuted) {
    Fixture f;
    for (int i = 0; i < static_cast<int>(BusId::Count); ++i) {
        EXPECT_FLOAT_EQ(f.audio->getBusGain(static_cast<BusId>(i)), 1.0f);
        EXPECT_FALSE(f.audio->isBusMuted(static_cast<BusId>(i)));
    }
}

// Master mute must win regardless of bus state, and vice versa -- neither can un-silence the other.
TEST(AudioBusTest, MasterMuteOverridesAnUnmutedBus) {
    Fixture f;
    auto v = f.audio->play(f.mono, {.volume = 1.0f, .bus = BusId::SFX});
    f.audio->setMuted(true);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.0f);
    f.audio->setBusGain(BusId::SFX, 1.0f);
    EXPECT_FLOAT_EQ(f.backend->voice(v)->params.gain, 0.0f) << "a bus change must not defeat the master mute";
}
