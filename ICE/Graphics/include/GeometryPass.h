#pragma once

#include <Entity.h>
#include <GPURegistry.h>

#include <Eigen/Dense>
#include <vector>

#include "Framebuffer.h"
#include "GraphicsFactory.h"
#include "RenderCommand.h"
#include "RenderPass.h"

namespace ICE {
class GeometryPass : public RenderPass {
   public:
    GeometryPass(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory,
                 const std::shared_ptr<GPURegistry>& gpu_registry, const FrameBufferFormat& format);
    void submit(std::vector<RenderCommand>* commands) { m_render_queue = commands; }

    // Bind this pass's own framebuffer, clear it, and draw the submitted commands into it.
    void execute() override;

    // Draw the submitted commands into whatever target is currently bound, without binding or
    // clearing one. This is what backs PassContext::drawScene, letting a graph pass replay the
    // visible set into its own target.
    //
    // `override_shader` replaces every command's shader and skips material uniform application:
    // an override (a depth-only shadow shader, say) declares its own inputs and would only pay for
    // texture binds it never reads. Per-command state, mesh binding and bone palettes still apply.
    void drawInto(ShaderProgram* override_shader = nullptr);
    std::shared_ptr<Framebuffer> getResult() const;
    void resize(int w, int h);

   private:
    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<GraphicsFactory> m_factory;
    std::shared_ptr<GPURegistry> m_gpu_registry;
    std::shared_ptr<Framebuffer> m_framebuffer;
    // Reused every draw for per-instance data instead of allocating a fresh GL buffer
    // per command per frame.
    std::shared_ptr<VertexBuffer> m_instance_buffer;
    // Reused scratch buffer for packing a skinned mesh's bone palette into a contiguous,
    // id-indexed array for a single glUniformMatrix4fv upload.
    std::vector<Eigen::Matrix4f> m_bone_palette;
    std::vector<RenderCommand>* m_render_queue;
};
}  // namespace ICE