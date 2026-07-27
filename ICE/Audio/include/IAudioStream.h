#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

namespace ICE {

// An incremental source of PCM, for sounds too long to hold in memory (music, ambience, dialogue).
// Where AudioClip is the whole decoded buffer, this hands out frames a chunk at a time.
//
// THREADING: a stream is driven by exactly one decode job at a time (the backend guarantees this
// with a single in-flight guard per voice), so implementations need no internal locking -- but they
// must not assume the calls all come from the *same* thread, since successive jobs may run on
// different workers. Plain member state is fine; thread_local or thread-affine handles are not.
class IAudioStream {
   public:
    virtual ~IAudioStream() = default;

    virtual uint32_t getChannels() const = 0;
    virtual uint32_t getSampleRate() const = 0;
    // Total frames in the source, or 0 when the format cannot report it cheaply.
    virtual uint64_t getTotalFrames() const = 0;

    // Read up to `maxFrames` interleaved frames into `out` (which must hold
    // maxFrames * getChannels() int16s). Returns the number of frames actually written; a short
    // read or 0 means end of stream.
    virtual uint64_t read(int16_t* out, uint64_t maxFrames) = 0;

    // Seek back to the start. This is how looping is done for streamed sounds -- AL_LOOPING cannot
    // be used on a queued-buffer source, since it would loop the individual queued buffer rather
    // than the underlying sound.
    virtual void rewind() = 0;

    virtual bool isValid() const = 0;
};

// Open a WAV/MP3/FLAC/OGG file for incremental decoding. Returns nullptr if the file cannot be
// opened or its extension is unsupported.
std::shared_ptr<IAudioStream> OpenAudioFileStream(const std::filesystem::path& file);

}  // namespace ICE
