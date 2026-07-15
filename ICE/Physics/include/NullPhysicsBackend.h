#pragma once

#include "IPhysicsBackend.h"

namespace ICE {

// No-op reference backend: satisfies the physics seam so an application can run with physics
// "attached" but inert, and serves as the template a real backend is written against.
class NullPhysicsBackend : public IPhysicsBackend {
   public:
    void initialize() override {}
    void step(double /*delta*/) override {}
};

}  // namespace ICE
