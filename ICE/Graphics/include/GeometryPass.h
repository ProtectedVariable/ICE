#pragma once

#include <GPURegistry.h>

#include <Eigen/Dense>
#include <memory>
#include <vector>

#include "Framebuffer.h"
#include "GraphicsFactory.h"
#include "RenderCommand.h"
#include "RenderFeature.h"

namespace ICE {

// Draws the frame's opaque + transparent geometry into the scene-colour target.
//
// As of the scriptable-pipeline migration (Phase 3) this is an ordinary render pass: it CREATES the
// scene-colour framebuffer -- the graph owns and allocates it -- and reads the sorted visible set
// from the frame context, instead of owning its own framebuffer and being driven directly by the
// renderer. Its output handle (color()) is threaded to the passes that read it (features, present).
class GeometryPass : public IRenderPass {
   public:
    GeometryPass(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory,
                 const std::shared_ptr<GPURegistry>& gpu_registry);

    const char* name() const override { return "geometry"; }

    // Create the scene-colour target (graph-allocated, at the current render size) and write it.
    void setup(RenderGraphBuilder& builder) override;

    // Clear the graph-bound target and draw the frame context's visible set into it.
    void execute(PassContext& ctx) override;

    // The scene-colour handle this pass created; valid after setup(), for the passes that read it.
    // Regenerated on every rebuild (handles are per-compile).
    RenderResourceHandle<Framebuffer> color() const { return m_color; }

    // The size the scene-colour target is created at. Set by the renderer (from its viewport) before
    // the graph is rebuilt.
    void setRenderSize(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;
    }

   private:
    // Draw a command queue into the currently bound target (state-cached; instance/bone reuse).
    void drawCommands(const std::vector<RenderCommand>& queue);

    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<GraphicsFactory> m_factory;
    std::shared_ptr<GPURegistry> m_gpu_registry;
    // Reused every draw for per-instance data instead of allocating a fresh GL buffer per command.
    std::shared_ptr<VertexBuffer> m_instance_buffer;
    // Reused scratch for packing a skinned mesh's bone palette into a contiguous id-indexed array.
    std::vector<Eigen::Matrix4f> m_bone_palette;

    RenderResourceHandle<Framebuffer> m_color;
    uint32_t m_width = 1;
    uint32_t m_height = 1;
};
}  // namespace ICE
