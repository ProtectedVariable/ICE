#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace ICE {
class RenderGraph;
class RendererAPI;
class RenderFeature;
struct FrameContext;

// Per-rebuild inputs a pipeline needs to assemble the frame's graph: the backend API, the per-frame
// context passes read at execute time, the current render size (for sizing targets), and the
// application-registered features to weave in.
struct PipelineContext {
    std::shared_ptr<RendererAPI> api;
    const FrameContext* frame = nullptr;
    uint32_t render_width = 1;
    uint32_t render_height = 1;
    const std::vector<std::unique_ptr<RenderFeature>>* features = nullptr;  // app-registered; may be null/empty
};

// Assembles the frame: declares the passes (and their resources) into `graph`. The renderer owns one
// pipeline and calls build() whenever the graph is (re)built -- there are no privileged passes; even
// geometry and present are declared here. Ships as ForwardPipeline; an application can replace it to
// define a custom frame (deferred shading, post-process chains, ...).
class Pipeline {
   public:
    virtual ~Pipeline() = default;
    virtual void build(RenderGraph& graph, const PipelineContext& ctx) = 0;
};
}  // namespace ICE
