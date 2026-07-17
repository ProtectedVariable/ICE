#pragma once

#include <RenderFeature.h>

#include "UIManager.h"

namespace ICE {

// Draws the UI over the finished scene as a render-graph pass (T5). It reads and writes the scene
// colour: reading orders it after the geometry pass, writing makes it contribute to the output (so
// the graph does not cull it) and marks the scene colour as this pass's target -- which the graph
// binds before execute(), so the UI composites straight onto the rendered frame.
//
// The manager is owned elsewhere (the engine) and must outlive the renderer that holds this pass.
class UIRenderPass : public IRenderPass {
   public:
    explicit UIRenderPass(UIManager* ui) : m_ui(ui) {}

    const char* name() const override { return "ui"; }

    void setup(RenderGraphBuilder& builder) override {
        builder.read(builder.sceneColor());   // run after geometry produced the scene
        builder.write(builder.sceneColor());  // draw over it (and stay uncalled)
    }

    void execute(PassContext& ctx) override {
        const auto& target = ctx.target();  // scene colour, already bound by the graph
        if (!m_ui || !target) {
            return;
        }
        m_ui->render(ctx.api(), static_cast<int>(target->getFormat().width), static_cast<int>(target->getFormat().height));
    }

   private:
    UIManager* m_ui;
};
}  // namespace ICE
