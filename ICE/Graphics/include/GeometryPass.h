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
    std::shared_ptr<Framebuffer> m_framebuffer;
    std::vector<RenderCommand>* m_render_queue;
};
}  // namespace ICE