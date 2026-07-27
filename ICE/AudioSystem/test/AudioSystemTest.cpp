#include <gtest/gtest.h>

#include <AudioClip.h>
#include <AudioDecoder.h>
#include <AudioSystem.h>

#include <cmath>
#include <memory>

#include "MockAudioBackend.h"

using namespace ICE;

namespace {

// A registry with an audio system wired to a mock backend, plus one mono clip to play.
struct Fixture {
    std::shared_ptr<AssetBank> bank = std::make_shared<AssetBank>();
    std::shared_ptr<MockAudioBackend> backend = std::make_shared<MockAudioBackend>(8);
    std::unique_ptr<AudioEngine> audio;
    std::shared_ptr<Registry> registry = std::make_shared<Registry>();
    std::shared_ptr<AudioSystem> system;
    AssetUID clip = NO_ASSET_ID;

    Fixture() {
        backend->initialize({});
        bank->addAsset<AudioClip>("mono", std::make_shared<AudioClip>(std::vector<int16_t>(100, 0), 1, 44100));
        clip = bank->getUID(AssetPath::WithTypePrefix<AudioClip>("mono"));
        audio = std::make_unique<AudioEngine>(backend, bank);
        system = std::make_shared<AudioSystem>(registry, audio.get());
        registry->addSystem(system);
    }

    // Create an entity with a transform at `position`. No parent matrix is needed: it defaults to
    // identity and getWorldMatrix() recomputes lazily whenever a setter marks the transform dirty,
    // which is what SceneGraphSystem relies on in a real frame too.
    Entity entityAt(const Eigen::Vector3f& position) {
        Entity e = registry->createEntity();
        registry->addComponent(e, TransformComponent(position));
        return e;
    }

    void moveTo(Entity e, const Eigen::Vector3f& position) { registry->getComponent<TransformComponent>(e)->setPosition(position); }

    AudioSourceComponent* source(Entity e) { return registry->getComponent<AudioSourceComponent>(e); }
};

constexpr double kFrame = 1.0 / 60.0;

}  // namespace

// --- Listener selection -------------------------------------------------------------------------

TEST(AudioSystemTest, FallbackListenerIsUsedWhenNoListenerComponentExists) {
    Fixture f;
    Entity camera = f.entityAt({1.0f, 2.0f, 3.0f});
    f.system->setListenerEntity(camera);

    f.system->update(kFrame);

    EXPECT_EQ(f.system->getActiveListener(), camera);
    EXPECT_FLOAT_EQ(f.backend->lastListener.position.y(), 2.0f);
}

TEST(AudioSystemTest, ExplicitListenerComponentOverridesTheCameraFallback) {
    Fixture f;
    Entity camera = f.entityAt({100.0f, 0.0f, 0.0f});
    Entity ears = f.entityAt({5.0f, 0.0f, 0.0f});
    f.registry->addComponent(ears, AudioListenerComponent{});
    f.system->setListenerEntity(camera);

    f.system->update(kFrame);

    EXPECT_EQ(f.system->getActiveListener(), ears) << "an explicit listener must win over the camera";
    EXPECT_FLOAT_EQ(f.backend->lastListener.position.x(), 5.0f);
}

TEST(AudioSystemTest, InactiveListenerComponentFallsBackToTheCamera) {
    Fixture f;
    Entity camera = f.entityAt({100.0f, 0.0f, 0.0f});
    Entity ears = f.entityAt({5.0f, 0.0f, 0.0f});
    AudioListenerComponent listener;
    listener.active = false;
    f.registry->addComponent(ears, listener);
    f.system->setListenerEntity(camera);

    f.system->update(kFrame);

    EXPECT_EQ(f.system->getActiveListener(), camera);
}

TEST(AudioSystemTest, ListenerOrientationComesFromTheWorldMatrix) {
    Fixture f;
    Entity camera = f.entityAt({0.0f, 0.0f, 0.0f});
    // Yaw 90 degrees about +Y: the -Z forward axis should swing to -X.
    f.registry->getComponent<TransformComponent>(camera)->setRotation(
        Eigen::Quaternionf(Eigen::AngleAxisf(static_cast<float>(M_PI) / 2.0f, Eigen::Vector3f::UnitY())));
    f.system->setListenerEntity(camera);

    f.system->update(kFrame);

    const auto& forward = f.backend->lastListener.forward;
    const auto& up = f.backend->lastListener.up;
    EXPECT_NEAR(forward.x(), -1.0f, 1e-4f);
    EXPECT_NEAR(forward.z(), 0.0f, 1e-4f);
    EXPECT_NEAR(up.y(), 1.0f, 1e-4f) << "up must remain +Y under a yaw";
}

TEST(AudioSystemTest, NoListenerIsNotFatal) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.playOnAwake = true;
    f.registry->addComponent(e, source);

    f.system->update(kFrame);  // no listener entity set at all

    EXPECT_EQ(f.system->getActiveListener(), NULL_ENTITY);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 1u) << "sources should still play without a listener";
}

// --- Source lifecycle ---------------------------------------------------------------------------

TEST(AudioSystemTest, PlayOnAwakeStartsExactlyOnce) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.playOnAwake = true;
    f.registry->addComponent(e, source);

    f.system->update(kFrame);
    ASSERT_EQ(f.audio->getActiveVoiceCount(), 1u);
    VoiceHandle first{f.source(e)->voice_index, f.source(e)->voice_generation};

    // Let the one-shot finish and settle.
    f.backend->finish(first);
    f.audio->update(kFrame);
    f.system->update(kFrame);
    EXPECT_EQ(f.source(e)->state, AudioSourceState::Stopped);

    // It must NOT re-arm: playOnAwake is a one-time event, not a loop.
    for (int i = 0; i < 5; ++i) {
        f.system->update(kFrame);
    }
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);
}

// The bug this guards: a finished one-shot has its voice reclaimed by AudioEngine, so the source
// sees state == Playing with no voice and would restart it every single frame, forever.
TEST(AudioSystemTest, FinishedOneShotSettlesToStoppedInsteadOfRestarting) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    f.registry->addComponent(e, AudioSourceComponent(f.clip));
    f.source(e)->play();

    f.system->update(kFrame);
    VoiceHandle voice{f.source(e)->voice_index, f.source(e)->voice_generation};
    ASSERT_TRUE(voice.valid());

    f.backend->finish(voice);
    f.audio->update(kFrame);
    f.system->update(kFrame);

    EXPECT_EQ(f.source(e)->state, AudioSourceState::Stopped);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);

    // Many frames later, still silent.
    for (int i = 0; i < 20; ++i) {
        f.system->update(kFrame);
        f.audio->update(kFrame);
    }
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);
    EXPECT_EQ(f.backend->uploadCount, 1);
}

TEST(AudioSystemTest, ReplayAfterFinishingWorks) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    f.registry->addComponent(e, AudioSourceComponent(f.clip));

    f.source(e)->play();
    f.system->update(kFrame);
    VoiceHandle first{f.source(e)->voice_index, f.source(e)->voice_generation};
    f.backend->finish(first);
    f.audio->update(kFrame);
    f.system->update(kFrame);
    ASSERT_EQ(f.source(e)->state, AudioSourceState::Stopped);

    // A fresh request must be honoured -- the "already finished" latch has to reset.
    f.source(e)->play();
    f.system->update(kFrame);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 1u);
}

TEST(AudioSystemTest, DirectStateAssignmentBehavesLikePlay) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    f.registry->addComponent(e, AudioSourceComponent(f.clip));

    // Gameplay code that sets the field rather than calling play() must work identically.
    f.source(e)->state = AudioSourceState::Playing;
    f.system->update(kFrame);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 1u);
}

TEST(AudioSystemTest, StoppingASourceReleasesItsVoice) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    f.registry->addComponent(e, AudioSourceComponent(f.clip));
    f.source(e)->play();
    f.system->update(kFrame);
    ASSERT_EQ(f.audio->getActiveVoiceCount(), 1u);

    f.source(e)->stop();
    f.system->update(kFrame);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);
    EXPECT_FALSE(VoiceHandle({f.source(e)->voice_index, f.source(e)->voice_generation}).valid());
}

TEST(AudioSystemTest, PauseAndResumeKeepTheSameVoice) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.loop = true;  // so it cannot end on its own mid-test
    f.registry->addComponent(e, source);
    f.source(e)->play();
    f.system->update(kFrame);
    VoiceHandle voice{f.source(e)->voice_index, f.source(e)->voice_generation};

    f.source(e)->pause();
    f.system->update(kFrame);
    EXPECT_EQ(f.backend->voice(voice)->state, PlaybackState::Paused);

    f.source(e)->play();
    f.system->update(kFrame);
    EXPECT_EQ(f.backend->voice(voice)->state, PlaybackState::Playing);
    VoiceHandle after{f.source(e)->voice_index, f.source(e)->voice_generation};
    EXPECT_EQ(voice, after) << "pause/resume must not recycle the voice";
}

TEST(AudioSystemTest, SourceWithNoClipIsSilentAndHarmless) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    f.registry->addComponent(e, AudioSourceComponent{});  // NO_ASSET_ID
    f.source(e)->play();
    f.system->update(kFrame);
    EXPECT_EQ(f.audio->getActiveVoiceCount(), 0u);
}

// --- Positioning and Doppler --------------------------------------------------------------------

TEST(AudioSystemTest, SourcePositionTracksTheEntityWorldTransform) {
    Fixture f;
    Entity listener = f.entityAt({0.0f, 0.0f, 0.0f});
    f.system->setListenerEntity(listener);

    Entity e = f.entityAt({3.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.loop = true;
    f.registry->addComponent(e, source);
    f.source(e)->play();
    f.system->update(kFrame);

    VoiceHandle voice{f.source(e)->voice_index, f.source(e)->voice_generation};
    EXPECT_FLOAT_EQ(f.backend->voice(voice)->params.position.x(), 3.0f);

    f.moveTo(e, {9.0f, 0.0f, 0.0f});
    f.system->update(kFrame);
    EXPECT_FLOAT_EQ(f.backend->voice(voice)->params.position.x(), 9.0f) << "a moving entity must move its sound";
}

TEST(AudioSystemTest, VelocityIsZeroOnTheFirstFrameThenReflectsMotion) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.loop = true;
    f.registry->addComponent(e, source);
    f.source(e)->play();

    f.system->update(kFrame);
    VoiceHandle voice{f.source(e)->voice_index, f.source(e)->voice_generation};
    // A spawn must not produce a huge bogus velocity (which would be an audible Doppler shriek).
    EXPECT_FLOAT_EQ(f.backend->voice(voice)->params.velocity.norm(), 0.0f);

    // Move +6 units over one 1/60s frame -> +360 units/second along X.
    f.moveTo(e, {6.0f, 0.0f, 0.0f});
    f.system->update(kFrame);
    EXPECT_NEAR(f.backend->voice(voice)->params.velocity.x(), 360.0f, 0.5f);
}

TEST(AudioSystemTest, RecedingSourceHasOppositeVelocitySignToApproaching) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.loop = true;
    f.registry->addComponent(e, source);
    f.source(e)->play();
    f.system->update(kFrame);
    VoiceHandle voice{f.source(e)->voice_index, f.source(e)->voice_generation};

    f.moveTo(e, {1.0f, 0.0f, 0.0f});
    f.system->update(kFrame);
    const float receding = f.backend->voice(voice)->params.velocity.x();

    f.moveTo(e, {0.0f, 0.0f, 0.0f});
    f.system->update(kFrame);
    const float approaching = f.backend->voice(voice)->params.velocity.x();

    EXPECT_GT(receding, 0.0f);
    EXPECT_LT(approaching, 0.0f);
}

TEST(AudioSystemTest, AuthoredFieldsReachTheDevice) {
    Fixture f;
    Entity e = f.entityAt({0.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.loop = true;
    source.volume = 0.25f;
    source.pitch = 1.5f;
    source.minDistance = 2.0f;
    source.maxDistance = 40.0f;
    source.rolloff = 0.5f;
    source.spatial = true;
    f.registry->addComponent(e, source);
    f.source(e)->play();
    f.system->update(kFrame);

    VoiceHandle voice{f.source(e)->voice_index, f.source(e)->voice_generation};
    const auto& params = f.backend->voice(voice)->params;
    EXPECT_FLOAT_EQ(params.gain, 0.25f);
    EXPECT_FLOAT_EQ(params.pitch, 1.5f);
    EXPECT_TRUE(params.looping);
    EXPECT_TRUE(params.spatial);
    EXPECT_FLOAT_EQ(params.minDistance, 2.0f);
    EXPECT_FLOAT_EQ(params.maxDistance, 40.0f);
    EXPECT_FLOAT_EQ(params.rolloff, 0.5f);
}

TEST(AudioSystemTest, NonSpatialSourceIsNotPositioned) {
    Fixture f;
    Entity e = f.entityAt({50.0f, 0.0f, 0.0f});
    AudioSourceComponent source(f.clip);
    source.spatial = false;
    source.loop = true;
    f.registry->addComponent(e, source);
    f.source(e)->play();
    f.system->update(kFrame);

    VoiceHandle voice{f.source(e)->voice_index, f.source(e)->voice_generation};
    EXPECT_FALSE(f.backend->voice(voice)->params.spatial);
}

// --- Downmix ------------------------------------------------------------------------------------

TEST(AudioDownmixTest, StereoAveragesToMono) {
    DecodedAudio audio;
    audio.channels = 2;
    audio.sampleRate = 44100;
    audio.samples = {100, 300, -200, 0};  // two frames: (100,300) and (-200,0)
    audio.frameCount = 2;

    DownmixToMono(audio);

    EXPECT_EQ(audio.channels, 1u);
    EXPECT_EQ(audio.frameCount, 2u);
    ASSERT_EQ(audio.samples.size(), 2u);
    EXPECT_EQ(audio.samples[0], 200);
    EXPECT_EQ(audio.samples[1], -100);
}

TEST(AudioDownmixTest, MonoIsUnchanged) {
    DecodedAudio audio;
    audio.channels = 1;
    audio.sampleRate = 8000;
    audio.samples = {1, 2, 3};
    audio.frameCount = 3;

    DownmixToMono(audio);

    EXPECT_EQ(audio.channels, 1u);
    ASSERT_EQ(audio.samples.size(), 3u);
    EXPECT_EQ(audio.samples[2], 3);
}

// Summing several near-full-scale int16 channels overflows 16 bits before the divide; the
// accumulator has to be wider.
TEST(AudioDownmixTest, LoudStereoDoesNotOverflow) {
    DecodedAudio audio;
    audio.channels = 2;
    audio.sampleRate = 44100;
    audio.samples = {32000, 32000, -32000, -32000};
    audio.frameCount = 2;

    DownmixToMono(audio);

    EXPECT_EQ(audio.samples[0], 32000);
    EXPECT_EQ(audio.samples[1], -32000);
}
