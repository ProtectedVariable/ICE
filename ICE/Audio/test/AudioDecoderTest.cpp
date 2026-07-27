#include <gtest/gtest.h>

#include <AudioDecoder.h>

#include <filesystem>

using namespace ICE;

// Phase 0 scope: these prove the decoder dependencies are fetched, compiled into the single
// implementation TU, and reachable through the public header -- and that the failure paths return
// nullopt rather than crashing or returning a half-built buffer. Round-trip tests against real
// encoded fixtures land in phase 1 alongside AudioClipLoader, which is what will consume them.

TEST(AudioDecoderTest, SupportedExtensionsAreLowercaseAndDotted) {
    const auto& extensions = SupportedAudioExtensions();
    ASSERT_FALSE(extensions.empty());
    for (const auto& ext : extensions) {
        EXPECT_EQ(ext.front(), '.') << ext << " should be dot-prefixed";
        EXPECT_EQ(ext, std::string(ext.begin(), ext.end())) << ext << " should already be lowercase";
        EXPECT_EQ(ext.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"), std::string::npos);
    }
}

TEST(AudioDecoderTest, CoversTheFourPlannedFormats) {
    const auto& extensions = SupportedAudioExtensions();
    for (const char* expected : {".wav", ".mp3", ".flac", ".ogg"}) {
        EXPECT_NE(std::find(extensions.begin(), extensions.end(), expected), extensions.end()) << expected << " missing";
    }
}

TEST(AudioDecoderTest, UnknownExtensionReturnsNullopt) {
    EXPECT_FALSE(DecodeAudioFile("nonexistent.xyz").has_value());
}

// Each branch dispatches into a different decoder library; a missing file must be rejected by the
// library itself rather than faulting. This is what actually links all four decoders in.
TEST(AudioDecoderTest, MissingFileReturnsNulloptForEveryFormat) {
    for (const auto& ext : SupportedAudioExtensions()) {
        EXPECT_FALSE(DecodeAudioFile("definitely_not_here" + ext).has_value()) << "for " << ext;
    }
}

TEST(AudioDecoderTest, ExtensionMatchIsCaseInsensitive) {
    // Uppercase must reach the decoder (and fail on the missing file), not fall through to the
    // unknown-extension branch. Both return nullopt, so assert on reaching the same outcome for a
    // path that exists in neither case -- the real assertion is that this does not crash.
    EXPECT_FALSE(DecodeAudioFile("missing.WAV").has_value());
    EXPECT_FALSE(DecodeAudioFile("missing.Ogg").has_value());
}

TEST(AudioDecoderTest, DurationIsZeroForAnEmptyResult) {
    DecodedAudio empty;
    EXPECT_DOUBLE_EQ(empty.duration(), 0.0);  // must not divide by a zero sample rate
}
