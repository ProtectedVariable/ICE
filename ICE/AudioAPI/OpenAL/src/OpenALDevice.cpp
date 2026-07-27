#include "OpenALDevice.h"

#include "ALCheck.h"

namespace ICE {

struct OpenALDevice::Impl {
    ALCdevice* device = nullptr;
    ALCcontext* context = nullptr;
    OpenALDeviceInfo info;
};

OpenALDevice::OpenALDevice() : m_impl(std::make_unique<Impl>()) {}

OpenALDevice::~OpenALDevice() {
    close();
}

bool OpenALDevice::open() {
    if (m_impl->device != nullptr) {
        return true;  // idempotent
    }

    m_impl->device = alcOpenDevice(nullptr);  // default device
    if (m_impl->device == nullptr) {
        Logger::Log(Logger::WARNING, "Audio", "No OpenAL device available; audio will be silent.");
        return false;
    }

    m_impl->context = alcCreateContext(m_impl->device, nullptr);
    if (m_impl->context == nullptr || alcMakeContextCurrent(m_impl->context) == ALC_FALSE) {
        Logger::Log(Logger::ERROR, "Audio", "Opened an OpenAL device but could not create/attach its context.");
        close();
        return false;
    }

    auto& info = m_impl->info;
    const ALCchar* name = alcGetString(m_impl->device, ALC_ALL_DEVICES_SPECIFIER);
    info.deviceName = name != nullptr ? name : "unknown";
    const ALchar* version = alGetString(AL_VERSION);
    info.version = version != nullptr ? version : "unknown";
    const ALchar* renderer = alGetString(AL_RENDERER);
    info.renderer = renderer != nullptr ? renderer : "unknown";

    // Capabilities the later phases depend on. Probed once here so a missing extension is a
    // startup log line rather than a mystery at the point of use.
    info.hrtfAvailable = alcIsExtensionPresent(m_impl->device, "ALC_SOFT_HRTF") == ALC_TRUE;
    info.efxAvailable = alcIsExtensionPresent(m_impl->device, "ALC_EXT_EFX") == ALC_TRUE;

    // The hard ceiling on simultaneous voices -- this is why the voice pool and its stealing
    // policy are mandatory rather than an optimization (see the phase 1 VoicePool).
    alcGetIntegerv(m_impl->device, ALC_MONO_SOURCES, 1, &info.monoSources);
    alcGetIntegerv(m_impl->device, ALC_STEREO_SOURCES, 1, &info.stereoSources);

    Logger::Log(Logger::INFO, "Audio", "OpenAL device '%s' (%s, %s) -- %d mono / %d stereo sources, HRTF %s, EFX %s",
                info.deviceName.c_str(), info.version.c_str(), info.renderer.c_str(), info.monoSources, info.stereoSources,
                info.hrtfAvailable ? "yes" : "no", info.efxAvailable ? "yes" : "no");
    return true;
}

void OpenALDevice::close() {
    if (m_impl->context != nullptr) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_impl->context);
        m_impl->context = nullptr;
    }
    if (m_impl->device != nullptr) {
        alcCloseDevice(m_impl->device);
        m_impl->device = nullptr;
    }
}

bool OpenALDevice::isOpen() const {
    return m_impl->context != nullptr;
}

const OpenALDeviceInfo& OpenALDevice::info() const {
    return m_impl->info;
}

std::vector<std::string> OpenALDevice::enumerateDevices() {
    std::vector<std::string> devices;
    if (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") != ALC_TRUE) {
        return devices;
    }
    // A double-null-terminated list of null-terminated strings.
    const ALCchar* list = alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
    if (list == nullptr) {
        return devices;
    }
    for (const ALCchar* entry = list; *entry != '\0';) {
        devices.emplace_back(entry);
        entry += devices.back().size() + 1;
    }
    return devices;
}

}  // namespace ICE
