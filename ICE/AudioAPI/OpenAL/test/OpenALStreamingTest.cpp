#include <gtest/gtest.h>

#include <IAudioStream.h>
#include <JobScheduler.h>
#include <OpenALBackend.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

using namespace ICE;

namespace {

// A synthetic stream, so these tests exercise the queue/refill machinery without touching the
// filesystem. `blockFor` lets a test hold a decode in progress, which is how the stop-during-refill
// race is made deterministic instead of hoped for.
class FakeStream : public IAudioStream {
   public:
    explicit FakeStream(uint64_t totalFrames, uint32_t channels = 1, uint32_t rate = 8000)
        : m_total(totalFrames),
          m_channels(channels),
          m_rate(rate) {}

    uint32_t getChannels() const override { return m_channels; }
    uint32_t getSampleRate() const override { return m_rate; }
    uint64_t getTotalFrames() const override { return m_total; }
    bool isValid() const override { return true; }

    uint64_t read(int16_t* out, uint64_t maxFrames) override {
        ++readCalls;
        if (blockFor.load() > 0) {
            inRead.store(true);
            std::this_thread::sleep_for(std::chrono::milliseconds(blockFor.load()));
            inRead.store(false);
        }
        const uint64_t remaining = m_total > m_pos ? m_total - m_pos : 0;
        const uint64_t got = std::min(remaining, maxFrames);
        for (uint64_t i = 0; i < got * m_channels; ++i) {
            out[i] = 0;
        }
        m_pos += got;
        return got;
    }

    void rewind() override {
        ++rewinds;
        m_pos = 0;
    }

    std::atomic<int> blockFor{0};   // ms to stall inside read()
    std::atomic<bool> inRead{false};
    std::atomic<int> readCalls{0};
    std::atomic<int> rewinds{0};

   private:
    uint64_t m_total;
    uint64_t m_pos = 0;
    uint32_t m_channels;
    uint32_t m_rate;
};

// These need a real device. CI usually has none, and that is a legitimate environment rather than a
// failure -- the null backend covers that path in the audio suite.
std::shared_ptr<OpenALBackend> makeBackend() {
    auto backend = std::make_shared<OpenALBackend>();
    if (!backend->initialize({})) {
        return nullptr;
    }
    return backend;
}

#define REQUIRE_DEVICE(backend)                                            \
    if ((backend) == nullptr) {                                            \
        GTEST_SKIP() << "no audio device available on this machine";       \
    }

}  // namespace

TEST(OpenALStreamingTest, AcquiresAStreamingVoiceAndPrimesIt) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);

    auto stream = std::make_shared<FakeStream>(8000 * 5);
    VoiceDesc desc;
    desc.params.spatial = false;

    VoiceHandle voice = backend->acquireStreamingVoice(stream, desc);
    ASSERT_TRUE(voice.valid());
    EXPECT_GT(stream->readCalls.load(), 0) << "the queue should be primed before playback starts";
    EXPECT_EQ(backend->activeVoiceCount(), 1u);

    backend->releaseVoice(voice);
    EXPECT_EQ(backend->activeVoiceCount(), 0u);
    backend->shutdown();
}

TEST(OpenALStreamingTest, RejectsAnInvalidStream) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);
    EXPECT_FALSE(backend->acquireStreamingVoice(nullptr, VoiceDesc{}).valid());
    backend->shutdown();
}

// A streaming voice must not be reported inactive just because the source momentarily ran dry;
// AudioEngine would reclaim it and the music would vanish on the first hitch.
TEST(OpenALStreamingTest, StaysActiveWhileTheStreamHasMoreAudio) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);

    auto stream = std::make_shared<FakeStream>(8000 * 30);  // 30s: far from exhausted
    VoiceDesc desc;
    desc.params.spatial = false;
    VoiceHandle voice = backend->acquireStreamingVoice(stream, desc);
    ASSERT_TRUE(voice.valid());

    backend->setVoiceState(voice, PlaybackState::Playing);
    for (int i = 0; i < 5; ++i) {
        backend->update(0.016);
        EXPECT_TRUE(backend->isVoiceActive(voice)) << "iteration " << i;
    }
    backend->releaseVoice(voice);
    backend->shutdown();
}

TEST(OpenALStreamingTest, LoopingRewindsRatherThanEnding) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);

    // Shorter than one chunk, so the very first fill has to wrap.
    auto stream = std::make_shared<FakeStream>(100);
    VoiceDesc desc;
    desc.params.spatial = false;
    desc.params.looping = true;

    VoiceHandle voice = backend->acquireStreamingVoice(stream, desc);
    ASSERT_TRUE(voice.valid());
    EXPECT_GT(stream->rewinds.load(), 0) << "a looping stream shorter than a chunk must rewind to fill it";

    backend->releaseVoice(voice);
    backend->shutdown();
}

TEST(OpenALStreamingTest, NonLoopingStreamEventuallyGoesInactive) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);

    auto stream = std::make_shared<FakeStream>(50);  // a few ms of audio
    VoiceDesc desc;
    desc.params.spatial = false;
    VoiceHandle voice = backend->acquireStreamingVoice(stream, desc);
    ASSERT_TRUE(voice.valid());
    backend->setVoiceState(voice, PlaybackState::Playing);

    bool went_inactive = false;
    for (int i = 0; i < 200 && !went_inactive; ++i) {
        backend->update(0.016);
        went_inactive = !backend->isVoiceActive(voice);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    EXPECT_TRUE(went_inactive) << "a finished, non-looping stream must not stay active forever";

    backend->releaseVoice(voice);
    backend->shutdown();
}

// The concurrency case the design is built around: releasing a voice while a decode job is running
// must not block, deadlock, or use freed state. The job holds its own shared_ptr to the stream
// state, so release just cancels and walks away.
TEST(OpenALStreamingTest, ReleaseDuringAnInFlightDecodeDoesNotDeadlock) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);

    auto scheduler = std::make_shared<JobScheduler>(2);
    backend->setScheduler(scheduler);

    auto stream = std::make_shared<FakeStream>(8000 * 60);
    VoiceDesc desc;
    desc.params.spatial = false;

    VoiceHandle voice = backend->acquireStreamingVoice(stream, desc);
    ASSERT_TRUE(voice.valid());
    backend->setVoiceState(voice, PlaybackState::Playing);

    // Make the next decode slow, then kick one off and tear the voice down while it runs.
    stream->blockFor.store(150);
    backend->update(0.016);

    const auto start = std::chrono::steady_clock::now();
    backend->releaseVoice(voice);
    const auto elapsed = std::chrono::steady_clock::now() - start;

    EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count(), 100)
        << "releaseVoice must not wait for the decode job to finish";
    EXPECT_EQ(backend->activeVoiceCount(), 0u);

    // Let the orphaned job finish against its own (still-alive) state before the scheduler dies.
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    backend->shutdown();
}

TEST(OpenALStreamingTest, DecodesOnTheSchedulerWhenOneIsAttached) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);

    auto scheduler = std::make_shared<JobScheduler>(2);
    backend->setScheduler(scheduler);

    auto stream = std::make_shared<FakeStream>(8000 * 60);
    VoiceDesc desc;
    desc.params.spatial = false;
    VoiceHandle voice = backend->acquireStreamingVoice(stream, desc);
    ASSERT_TRUE(voice.valid());

    const int before = stream->readCalls.load();
    backend->update(0.016);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_GT(stream->readCalls.load(), before) << "the detached decode job should have run";

    backend->releaseVoice(voice);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    backend->shutdown();
}

// Many streaming voices at once: buffers and sources must all come back, with no leak into the
// driver and no cross-talk between voices' buffer queues.
TEST(OpenALStreamingTest, ManyStreamingVoicesAcquireAndReleaseCleanly) {
    auto backend = makeBackend();
    REQUIRE_DEVICE(backend);

    std::vector<VoiceHandle> voices;
    for (int i = 0; i < 8; ++i) {
        auto stream = std::make_shared<FakeStream>(8000 * 10);
        VoiceDesc desc;
        desc.params.spatial = false;
        VoiceHandle v = backend->acquireStreamingVoice(stream, desc);
        ASSERT_TRUE(v.valid()) << "voice " << i;
        voices.push_back(v);
    }
    EXPECT_EQ(backend->activeVoiceCount(), 8u);

    for (int i = 0; i < 3; ++i) {
        backend->update(0.016);
    }
    for (VoiceHandle v : voices) {
        backend->releaseVoice(v);
    }
    EXPECT_EQ(backend->activeVoiceCount(), 0u);

    // The sources must be back in the pool: a fresh voice still acquires.
    auto stream = std::make_shared<FakeStream>(8000);
    VoiceHandle again = backend->acquireStreamingVoice(stream, VoiceDesc{});
    EXPECT_TRUE(again.valid());
    backend->releaseVoice(again);
    backend->shutdown();
}
