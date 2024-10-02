#pragma once

#include <Entity.h>

#include "Framebuffer.h"
#include "GraphicsFactory.h"
#include "RenderCommand.h"
#include "RenderPass.h"

namespace ICE {
class GeometryPass : public RenderPass {
   public:
    GeometryPass(const std::shared_ptr<RendererAPI>& api, const GraphicsFactory& factory, const FrameBufferFormat& format);
    void submit(const std::vector<RenderCommand>& commands) { m_render_queue.emplace(commands); }
    void execute() override;

   private:
    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<Framebuffer> m_framebuffer;
    std::optional<std::vector<RenderCommand>&> m_render_queue;
};
}  // namespace ICE