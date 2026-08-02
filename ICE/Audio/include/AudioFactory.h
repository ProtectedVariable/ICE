#pragma once

#include <memory>
#include <string>

#include "IAudioBackend.h"

namespace ICE {

// Backend-selection seam, the audio counterpart of GraphicsFactory. Core depends on this, never on
// a concrete backend, so swapping OpenAL for something else is a one-line change at the
// composition point.
class AudioFactory {
   public:
    virtual ~AudioFactory() = default;
    virtual std::shared_ptr<IAudioBackend> createBackend() const = 0;
    virtual std::string name() const = 0;
};

}  // namespace ICE
