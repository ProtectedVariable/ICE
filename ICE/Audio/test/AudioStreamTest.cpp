#include <gtest/gtest.h>

#include <AudioClipLoader.h>
#include <IAudioStream.h>

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace ICE;
namespace fs = std::filesystem;

namespace {

// A WAV whose sample values encode their own frame index, so a test can tell exactly where in the
// stream a chunk came from -- which is what makes rewind and loop-continuity checkable rather than
// just "some audio came back".
fs::path writeRampWav(const std::string& name, uint32_t frames, uint16_t channels = 1, uint32_t rate = 8000) {
    fs::path path = fs::temp_directory_path() / name;
    std::vector<int16_t> pcm(static_cast<std::size_t>(frames) * channels);
    for (uint32_t i = 0; i < frames; ++i) {
        for (uint16_t c = 0; c < channels; ++c) {
            pcm[i * channels + c] = static_cast<int16_t>(i % 30000);
        }
    }
    const uint32_t data_bytes = static_cast<uint32_t>(pcm.size() * 2);

    std::ofstream out(path, std::ios::binary);
    auto u32 = [&](uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    auto u16 = [&](uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };
    out.write("RIFF", 4);
    u32(36 + data_bytes);
    out.write("WAVE", 4);
    out.write("fmt ", 4);
    u32(16);
    u16(1);  // PCM
    u16(channels);
    u32(rate);
    u32(rate * channels * 2);
    u16(static_cast<uint16_t>(channels * 2));
    u16(16);
    out.write("data", 4);
    u32(data_bytes);
    out.write(reinterpret_cast<const char*>(pcm.data()), data_bytes);
    return path;
}

// Owns a generated WAV and deletes it when the test ends.
//
// DECLARE THIS BEFORE ANY STREAM THAT READS IT. Locals destruct in reverse declaration order, so
// declaring the TempWav first means the decoder handle is closed before the file is deleted.
// Windows refuses to delete a file that still has an open handle; POSIX allows it, which is why
// deleting the file with the stream still alive passed locally and failed on the Windows runner.
//
// Deletion is also non-throwing: failing to clean up a temp file is not a reason to fail a test
// that already verified what it set out to (and on Windows an AV scanner can hold a handle open
// briefly regardless of what this process does).
struct TempWav {
    fs::path path;

    TempWav(const std::string& name, uint32_t frames, uint16_t channels = 1, uint32_t rate = 8000)
        : path(writeRampWav(name, frames, channels, rate)) {}

    ~TempWav() {
        std::error_code ec;
        fs::remove(path, ec);
    }

    TempWav(const TempWav&) = delete;
    TempWav& operator=(const TempWav&) = delete;
};

}  // namespace

TEST(AudioStreamTest, ReportsFormatFromTheHeader) {
    TempWav wav("ice_stream_fmt.wav", 1000, 2, 22050);
    auto stream = OpenAudioFileStream(wav.path);
    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->getChannels(), 2u);
    EXPECT_EQ(stream->getSampleRate(), 22050u);
    EXPECT_EQ(stream->getTotalFrames(), 1000u);
}

TEST(AudioStreamTest, ReadsIncrementallyInOrder) {
    TempWav wav("ice_stream_seq.wav", 500);
    auto stream = OpenAudioFileStream(wav.path);
    ASSERT_NE(stream, nullptr);

    std::vector<int16_t> buffer(100);
    EXPECT_EQ(stream->read(buffer.data(), 100), 100u);
    EXPECT_EQ(buffer[0], 0) << "first chunk starts at frame 0";
    EXPECT_EQ(buffer[99], 99);

    EXPECT_EQ(stream->read(buffer.data(), 100), 100u);
    EXPECT_EQ(buffer[0], 100) << "the second read must continue where the first stopped";
}

TEST(AudioStreamTest, ShortReadAtEndThenZero) {
    TempWav wav("ice_stream_eof.wav", 150);
    auto stream = OpenAudioFileStream(wav.path);
    ASSERT_NE(stream, nullptr);

    std::vector<int16_t> buffer(100);
    EXPECT_EQ(stream->read(buffer.data(), 100), 100u);
    EXPECT_EQ(stream->read(buffer.data(), 100), 50u) << "a partial final chunk";
    EXPECT_EQ(stream->read(buffer.data(), 100), 0u) << "exhausted";
}

// Looping a streamed sound is the decoder rewinding, so this is the loop point: after a rewind the
// very next sample must be frame 0 again, with no gap.
TEST(AudioStreamTest, RewindReturnsToTheStart) {
    TempWav wav("ice_stream_rewind.wav", 200);
    auto stream = OpenAudioFileStream(wav.path);
    ASSERT_NE(stream, nullptr);

    std::vector<int16_t> buffer(200);
    ASSERT_EQ(stream->read(buffer.data(), 200), 200u);
    EXPECT_EQ(stream->read(buffer.data(), 10), 0u);

    stream->rewind();
    ASSERT_EQ(stream->read(buffer.data(), 10), 10u);
    EXPECT_EQ(buffer[0], 0) << "rewind must resume at frame 0";
    EXPECT_EQ(buffer[9], 9);
}

// The seamless-loop case: filling a chunk larger than what remains must wrap into the rewound
// stream and leave NO silence at the join.
TEST(AudioStreamTest, LoopFillAcrossTheEndHasNoGap) {
    const uint32_t total = 120;
    TempWav wav("ice_stream_loop.wav", total);
    auto stream = OpenAudioFileStream(wav.path);
    ASSERT_NE(stream, nullptr);

    // Mirrors the backend's loop-fill: read, and on a short read rewind and top the chunk up.
    std::vector<int16_t> chunk(200, -1);
    uint64_t got = stream->read(chunk.data(), 200);
    ASSERT_EQ(got, total);
    stream->rewind();
    while (got < 200) {
        const uint64_t more = stream->read(chunk.data() + got, 200 - got);
        ASSERT_GT(more, 0u);
        got += more;
    }

    EXPECT_EQ(got, 200u);
    EXPECT_EQ(chunk[total - 1], static_cast<int16_t>(total - 1)) << "last frame before the join";
    EXPECT_EQ(chunk[total], 0) << "the join must continue straight into frame 0, not silence";
    EXPECT_EQ(chunk[total + 1], 1);
}

TEST(AudioStreamTest, UnsupportedOrMissingFileReturnsNull) {
    EXPECT_EQ(OpenAudioFileStream("does_not_exist.wav"), nullptr);
    EXPECT_EQ(OpenAudioFileStream("thing.xyz"), nullptr);
}

// --- Loader threshold ---------------------------------------------------------------------------

TEST(AudioStreamTest, LoaderStreamsFilesAtOrAboveTheThreshold) {
    TempWav big("ice_stream_big.wav", 40000);  // ~80 KB of PCM
    const std::size_t original = AudioClipLoader::streamingThresholdBytes();

    AudioClipLoader loader;

    AudioClipLoader::setStreamingThresholdBytes(1024);  // force the streaming path
    auto streamed = loader.load({big.path});
    ASSERT_NE(streamed, nullptr);
    EXPECT_TRUE(streamed->isStreaming());
    EXPECT_TRUE(streamed->samples().empty()) << "a streaming clip holds no resident PCM";
    EXPECT_EQ(streamed->getChannels(), 1u);
    EXPECT_EQ(streamed->getFrameCount(), 40000u) << "length still comes from the header";

    AudioClipLoader::setStreamingThresholdBytes(100u * 1024 * 1024);  // force the resident path
    auto resident = loader.load({big.path});
    ASSERT_NE(resident, nullptr);
    EXPECT_FALSE(resident->isStreaming());
    EXPECT_EQ(resident->getFrameCount(), 40000u);
    EXPECT_FALSE(resident->samples().empty());

    AudioClipLoader::setStreamingThresholdBytes(original);
}
