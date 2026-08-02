#pragma once

#include <memory>

#include "GeometryPass.h"
#include "Pipeline.h"
#include "PresentPass.h"
#include "RenderFeature.h"

namespace ICE {

// The engine's default pipeline: forward geometry into the scene colour, then the app-registered
// features, then the present composite. This is the whole frame the renderer used to hardcode --
// now just one pipeline definition, replaceable by an application. It owns its built-in passes.
class ForwardPipeline : public Pipeline {
   public:
    ForwardPipeline(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory,
                    const std::shared_ptr<GPURegistry>& gpu_registry)
        : m_geometry(api, factory, gpu_registry) {}

    void build(RenderGraph& graph, const PipelineContext& ctx) override {
        // Geometry creates + writes the scene colour; its handle is threaded to everything that
        // reads it.
        m_geometry.setRenderSize(ctx.render_width, ctx.render_height);
        addPassToGraph(graph, m_geometry, {}, ctx.api, ctx.frame);
        const auto scene_color = m_geometry.color();

        // Application passes/features run in the middle, over the scene colour.
        if (ctx.features) {
            for (const auto& feature : *ctx.features) {
                addFeaturePasses(graph, *feature, scene_color, ctx.api, ctx.frame);
            }
        }

        // Present composites the scene colour to the backbuffer (declares the graph output).
        addPassToGraph(graph, m_present, scene_color, ctx.api, ctx.frame);
    }

   private:
    GeometryPass m_geometry;
    PresentPass m_present;
};
}  // namespace ICE
