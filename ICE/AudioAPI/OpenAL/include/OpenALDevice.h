#pragma once

#include <AudioTypes.h>

#include <memory>
#include <string>
#include <vector>

namespace ICE {

// Describes the device that was actually opened, for logging and for the editor's audio settings
// panel. Populated by OpenALDevice::open().
struct OpenALDeviceInfo {
    std::string deviceName;
    std::string version;
    std::string renderer;
    bool hrtfAvailable = false;
    bool efxAvailable = false;   // ALC_EXT_EFX -- reverb and occlusion filters (phase 5)
    int monoSources = 0;         // upper bound on simultaneously playing 3D voices
    int stereoSources = 0;
};

// RAII wrapper over the ALCdevice/ALCcontext pair. Deliberately exposes no OpenAL types: the
// OpenAL::OpenAL link is PRIVATE to this module, so <AL/al.h> must not leak into a public header.
//
// This is the seam the phase 1 OpenALBackend is built on. On its own it does nothing but open,
// interrogate and close the default device -- which is exactly what proves the dependency links
// and a device is reachable on each platform.
class OpenALDevice {
   public:
    OpenALDevice();
    ~OpenALDevice();

    OpenALDevice(const OpenALDevice&) = delete;
    OpenALDevice& operator=(const OpenALDevice&) = delete;

    // Open the default device and make its context current. Returns false if no device is
    // available -- the expected case on a headless CI machine, and the engine's cue to fall back
    // to the null backend rather than treating audio as fatal.
    //
    // The config's voice counts are passed as context attributes. This matters: OpenAL Soft's
    // DEFAULT context allocates 255 mono but only ONE stereo source, which would silently cap all
    // non-spatialized playback (music + UI together) at a single simultaneous sound.
    bool open(const AudioDeviceConfig& config = {});
    void close();

    bool isOpen() const;
    const OpenALDeviceInfo& info() const;

    // Devices the driver advertises. Empty if enumeration is unsupported.
    static std::vector<std::string> enumerateDevices();

   private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace ICE
