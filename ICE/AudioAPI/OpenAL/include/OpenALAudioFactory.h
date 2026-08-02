#pragma once

#include <AudioFactory.h>

#include "OpenALBackend.h"

namespace ICE {

// Concrete factory for the OpenAL backend, mirroring OpenGLFactory on the graphics side. Selecting
// a different audio backend later means handing the engine a different factory -- nothing in core
// names OpenAL.
class OpenALAudioFactory : public AudioFactory {
   public:
    std::shared_ptr<IAudioBackend> createBackend() const override { return std::make_shared<OpenALBackend>(); }
    std::string name() const override { return "OpenAL"; }
};

}  // namespace ICE
