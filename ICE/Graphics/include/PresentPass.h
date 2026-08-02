#pragma once

#include "Framebuffer.h"
#include "RenderFeature.h"
#include "ShaderProgram.h"
#include "VertexArray.h"

namespace ICE {

// The frame's final composite, as an ordinary render pass (scriptable-pipeline migration, Phase 2):
// it samples the scene colour and blits it full-screen to the backbuffer -- the window's default
// framebuffer, or the editor's render-to-texture target. Everything else (the present shader, the
// destination, the full-screen quad) comes from the FrameContext, so it depends on no renderer
// internals and could be swapped out or reordered by a custom pipeline like any other pass.
class PresentPass : public IRenderPass {
   public:
    const char* name() const override { return "present"; }

    void setup(RenderGraphBuilder& builder) override {
        m_color = builder.read(builder.sceneColor());  // ordered after everything that wrote the scene
        builder.presentsToBackbuffer();                // graph output; binds the real target in execute
    }

    void execute(PassContext& ctx) override {
        auto scene_fb = ctx.get(m_color);
        const FrameContext& f = ctx.frame();
        if (!scene_fb || !f.presentShader || !f.fullscreenQuad || !ctx.api()) {
            return;
        }
        // Destination: the editor's render-to-texture target if one is set, else the window's default
        // framebuffer. Size the viewport to it (the target for the editor, the scene size otherwise).
        if (f.outputTarget) {
            f.outputTarget->bind();
            ctx.api()->setViewport(0, 0, static_cast<int>(f.outputTarget->getFormat().width),
                                   static_cast<int>(f.outputTarget->getFormat().height));
        } else {
            ctx.api()->bindDefaultFramebuffer();
            ctx.api()->setViewport(0, 0, static_cast<int>(scene_fb->getFormat().width), static_cast<int>(scene_fb->getFormat().height));
        }
        ctx.api()->clear();
        f.presentShader->bind();
        scene_fb->bindAttachment(0);
        f.presentShader->loadInt("uTexture", 0);
        f.fullscreenQuad->bind();
        f.fullscreenQuad->getIndexBuffer()->bind();
        ctx.api()->renderVertexArray(f.fullscreenQuad);
    }

   private:
    RenderResourceHandle<Framebuffer> m_color;
};
}  // namespace ICE
