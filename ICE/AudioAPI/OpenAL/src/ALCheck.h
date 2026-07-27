#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>  // ALC_HRTF_SOFT and the other OpenAL Soft extension tokens
#include <Logger.h>

// alGetError() is a global, per-context error flag with no call-site association: an unchecked
// error silently persists and is then misattributed to whatever call happens to check next. Every
// al* call in this module goes through AL_CHECK so a failure names the operation that caused it.
//
// Retrofitting this is miserable, so it exists from the first call onward.
#define AL_CHECK(call)                                                                              \
    do {                                                                                            \
        (call);                                                                                     \
        if (ALenum _al_err = alGetError(); _al_err != AL_NO_ERROR) {                                \
            ICE::Logger::Log(ICE::Logger::ERROR, "Audio", "%s failed: %s (0x%x)", #call,            \
                             ICE::alErrorString(_al_err), _al_err);                                 \
        }                                                                                           \
    } while (0)

// Same idea for the ALC (context/device) layer, whose errors live on the device rather than the
// current context.
#define ALC_CHECK(device, call)                                                                     \
    do {                                                                                            \
        (call);                                                                                     \
        if (ALCenum _alc_err = alcGetError(device); _alc_err != ALC_NO_ERROR) {                     \
            ICE::Logger::Log(ICE::Logger::ERROR, "Audio", "%s failed: %s (0x%x)", #call,            \
                             ICE::alcErrorString(_alc_err), _alc_err);                              \
        }                                                                                           \
    } while (0)

namespace ICE {

inline const char* alErrorString(ALenum error) {
    switch (error) {
        case AL_NO_ERROR: return "AL_NO_ERROR";
        case AL_INVALID_NAME: return "AL_INVALID_NAME";
        case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
        default: return "unknown AL error";
    }
}

inline const char* alcErrorString(ALCenum error) {
    switch (error) {
        case ALC_NO_ERROR: return "ALC_NO_ERROR";
        case ALC_INVALID_DEVICE: return "ALC_INVALID_DEVICE";
        case ALC_INVALID_CONTEXT: return "ALC_INVALID_CONTEXT";
        case ALC_INVALID_ENUM: return "ALC_INVALID_ENUM";
        case ALC_INVALID_VALUE: return "ALC_INVALID_VALUE";
        case ALC_OUT_OF_MEMORY: return "ALC_OUT_OF_MEMORY";
        default: return "unknown ALC error";
    }
}

}  // namespace ICE
