#pragma once

#include <memory>
#include <vector>

#include "RenderCommand.h"

namespace ICE {
class RendererAPI;
class GraphicsFactory;
class GPURegistry;
class Camera;
class UniformBuffer;
class VertexArray;
class Framebuffer;
class ShaderProgram;

// Per-frame data + backend services handed to every render pass (through PassContext::frame()).
//
// The renderer builds one of these each frame and its address is stable, so the graph's compiled
// pass callbacks read *fresh* per-frame data through it every frame without a recompile. Passes read
// from it; they never own it. This is the conduit that lets passes reach the visible set and backend
// services without depending on the concrete renderer -- the groundwork for geometry/present
// becoming ordinary passes (see docs/scriptable_render_pipeline_plan.md).
//
// Phase 1 of that migration: populated by the renderer but not yet consumed by any shipped pass
// (geometry and present still run through the renderer's internal path). Fields are filled in as
// later phases need them; any left null simply have no consumer yet.
struct FrameContext {
    // --- Backend services ---
    RendererAPI* api = nullptr;          // draw calls + GL state
    GraphicsFactory* factory = nullptr;  // backend resource creation
    GPURegistry* gpu = nullptr;          // resolve mesh/shader/texture handles

    // --- Per-frame data ---
    Camera* camera = nullptr;                              // the frame's main view
    const std::vector<RenderCommand>* commands = nullptr;  // the sorted visible set

    // --- Shared GPU resources a pass may need ---
    UniformBuffer* cameraUBO = nullptr;      // a pass uploads the camera it draws from
    UniformBuffer* lightUBO = nullptr;
    // shared_ptr because RendererAPI::renderVertexArray takes one; for present / post-process passes.
    std::shared_ptr<VertexArray> fullscreenQuad;

    // --- Present ---
    // Destination: the editor's render-to-texture target, or null for the window's default
    // framebuffer. The full-screen composite shader the present pass blits with.
    Framebuffer* outputTarget = nullptr;
    ShaderProgram* presentShader = nullptr;
};
}  // namespace ICE
