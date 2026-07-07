#pragma once

#include <Entity.h>

#include "Framebuffer.h"
#include "GraphicsFactory.h"
#include "RenderCommand.h"
#include "RenderPass.h"

namespace ICE {
class GeometryPass : public RenderPass {
   public:
    GeometryPass(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory, const FrameBufferFormat& format);
    void submit(std::vector<RenderCommand>* commands) { m_render_queue = commands; }
    void execute() override;
    std::shared_ptr<Framebuffer> getResult() const;
    void resize(int w, int h);

   private:
    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<GraphicsFactory> m_factory;
    std::shared_ptr<Framebuffer> m_framebuffer;
    // Reused every draw for per-instance data instead of allocating a fresh GL buffer
    // per command per frame.
    std::shared_ptr<VertexBuffer> m_instance_buffer;
    std::vector<RenderCommand>* m_render_queue;
};
}  // namespace ICE