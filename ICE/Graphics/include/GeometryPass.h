#pragma once

#include "Framebuffer.h"
#include "GraphicsFactory.h"
#include "RenderPass.h"

namespace ICE {
class GeometryPass : public RenderPass {
   public:
    GeometryPass(const GraphicsFactory& factory);
    void execute() override;

   private:
    std::shared_ptr<Framebuffer> m_framebuffer;
};
}  // namespace ICE