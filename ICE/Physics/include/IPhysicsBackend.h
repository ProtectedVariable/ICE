#pragma once

namespace ICE {

// Engine-level physics seam, mirroring the RendererAPI abstraction: the engine drives an
// IPhysicsBackend each frame without knowing the concrete simulation (Bullet, Jolt, PhysX, ...). A
// new backend plugs in via ICEEngine::setPhysicsBackend with no change to core.
//
// Interface-first and deliberately minimal -- the body/collision/query API grows once there is a
// PhysicsComponent consumer to drive its shape. Defining the seam now keeps that later work from
// having to thread a backend through the engine after gameplay code exists.
class IPhysicsBackend {
   public:
    virtual ~IPhysicsBackend() = default;

    // Called once, when the backend is attached to the engine.
    virtual void initialize() = 0;

    // Advance the simulation by `delta` seconds. The engine runs this before the ECS systems each
    // frame, so gameplay sees this frame's simulation results.
    virtual void step(double delta) = 0;
};

}  // namespace ICE
