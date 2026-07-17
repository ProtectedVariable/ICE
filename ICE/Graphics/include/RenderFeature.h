//
// The public pass/feature seam of the render graph: application code contributes render passes and
// declares their resources with typed handles, without editing the renderer.
//
//   class OutlinePass : public IRenderPass {
//      public:
//       const char* name() const override { return "outline"; }
//       void setup(RenderGraphBuilder& builder) override {
//           m_scene = builder.read(builder.sceneColor());
//           builder.write(builder.sceneColor());   // contributes to the output -> not culled
//       }
//       void execute(PassContext& ctx) override { auto fb = ctx.get(m_scene); /* ... */ }
//      private:
//       RenderResourceHandle<Framebuffer> m_scene;
//   };
//
//   class OutlineFeature : public RenderFeature {
//      public:
//       OutlineFeature() { addPass<OutlinePass>(); }
//       const char* name() const override { return "outline"; }
//   };
//
//   renderer->setUseRenderGraph(true);
//   renderer->addFeature(std::make_unique<OutlineFeature>());
//

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Camera.h"
#include "GraphicsAPI.h"
#include "RenderGraph.h"

namespace ICE {

// The drawing the renderer performs on a pass's behalf, so a pass body is just draw calls instead
// of hand-rolled submission. Implemented by the renderer; passes reach it through PassContext.
class IPassDrawer {
   public:
    virtual ~IPassDrawer() = default;

    // Draw the frame's visible geometry from `camera` into the currently bound target.
    virtual void drawScene(Camera& camera, ShaderProgram* override_shader) = 0;

    // Draw a full-screen quad with `shader` into the currently bound target.
    virtual void fullscreen(ShaderProgram* shader) = 0;
};

// Setup-time API handed to a pass. Every resource a pass touches is named by a typed handle, so it
// can only refer to resources that were actually declared -- there is no string to mistype and no
// null physical resource to discover at draw time.
class RenderGraphBuilder {
   public:
    RenderGraphBuilder(RenderGraph& graph, RenderGraphPass& pass, RenderResourceHandle<Framebuffer> scene_color)
        : m_graph(graph),
          m_pass(pass),
          m_scene_color(scene_color) {}

    // Declare a new resource produced by this pass and return a typed handle to it. T selects the
    // resource type (see ResourceTypeOf) and fills in desc.type, so create<SomeUnsupportedType>()
    // is a compile error. An unnamed resource gets a name unique to this pass; naming one lets
    // other passes refer to the same resource.
    template<typename T>
    RenderResourceHandle<T> create(ResourceDescriptor desc) {
        desc.type = ResourceTypeOf<T>::value;  // the type parameter is authoritative
        if (desc.debug_name.empty()) {
            desc.debug_name = m_pass.getName() + "#" + std::to_string(m_next_unnamed++);
        }
        const auto index = m_graph.declareResource(desc.debug_name, desc);
        m_pass.create(desc.debug_name, desc);  // keeps dependency ordering + culling working
        return RenderResourceHandle<T>{index};
    }

    // Declare that this pass reads `handle`: it is ordered after whichever pass produces it.
    // Returns the handle so it can be stored in one line.
    template<typename T>
    RenderResourceHandle<T> read(RenderResourceHandle<T> handle) {
        m_pass.read(m_graph.resourceName(handle.index()));
        return handle;
    }

    // Declare that this pass writes `handle`: passes reading it are ordered after this one.
    //
    // Note this is also what keeps a pass alive: compile() culls any pass that does not contribute
    // (transitively) to the graph's output, so a pass that only writes a resource nobody reads is
    // silently dropped. A post-process feature stays live by writing sceneColor().
    template<typename T>
    RenderResourceHandle<T> write(RenderResourceHandle<T> handle) {
        m_pass.write(m_graph.resourceName(handle.index()));
        if constexpr (std::is_same_v<T, Framebuffer>) {
            // The first framebuffer a pass writes is its render target: the graph binds it (and
            // sizes the viewport to it) before execute(), and hands it back as ctx.target().
            if (!m_target.valid()) {
                m_target = handle;
            }
        }
        return handle;
    }

    // The pass's render target as declared by its first write() of a framebuffer; invalid if it
    // wrote none. Read by the renderer when wiring the pass -- passes use ctx.target() instead.
    RenderResourceHandle<Framebuffer> targetHandle() const { return m_target; }

    // The colour target the engine's geometry pass renders into: the conventional input (and
    // output) of a post-process feature. Imported by the renderer before any feature's setup runs.
    RenderResourceHandle<Framebuffer> sceneColor() const { return m_scene_color; }

    // The graph being built, for the rare pass that needs more than this builder exposes.
    RenderGraph& graph() { return m_graph; }

   private:
    RenderGraph& m_graph;
    RenderGraphPass& m_pass;
    RenderResourceHandle<Framebuffer> m_scene_color;
    RenderResourceHandle<Framebuffer> m_target;
    uint32_t m_next_unnamed = 0;
};

// Execute-time API handed to a pass: its target is already bound, its declared handles resolve
// here, and the renderer will draw for it -- so a pass body is draw calls, not setup boilerplate.
class PassContext {
   public:
    PassContext(RenderGraph& graph, const std::shared_ptr<RendererAPI>& api, IPassDrawer* drawer, std::shared_ptr<Framebuffer> target)
        : m_graph(graph),
          m_api(api),
          m_drawer(drawer),
          m_target(std::move(target)) {}

    // The physical resource behind a handle. Null while the graph leaves a resource virtual --
    // today that is TextureCube/Buffer, which have no descriptor-based creation yet.
    template<typename T>
    std::shared_ptr<T> get(RenderResourceHandle<T> handle) const {
        auto* resource = m_graph.resourceAt(handle.index());
        return resource ? resource->getPhysicalResourceAs<T>() : nullptr;
    }

    // This pass's render target (its first declared framebuffer write). Already bound, with the
    // viewport sized to it, before execute() is called -- you do not need to bind it yourself.
    // Null if the pass declared no framebuffer write.
    //
    // It is bound but deliberately NOT cleared: a pass that writes sceneColor() to draw an overlay
    // on top of the scene must not wipe it. If your pass owns its target (a shadow map, a bloom
    // buffer), clear it yourself first: ctx.api()->clear().
    const std::shared_ptr<Framebuffer>& target() const { return m_target; }

    // Draw the frame's visible geometry from `camera` into this pass's target. `override_shader`
    // replaces every command's shader and skips material uniforms, which is what a shadow pass
    // wants:
    //   ctx.drawScene(light_camera, depth_shader.get());
    // Passing a different camera does not disturb the frame's main camera for later passes.
    void drawScene(Camera& camera, ShaderProgram* override_shader = nullptr) {
        if (m_drawer) {
            m_drawer->drawScene(camera, override_shader);
        }
    }

    // Draw a full-screen quad with `shader` into this pass's target -- the post-process primitive.
    // Bind the inputs you sample on `shader` first:
    //   ctx.get(m_scene)->bindAttachment(0);
    //   shader->loadInt("uTexture", 0);
    //   ctx.fullscreen(shader.get());
    void fullscreen(ShaderProgram* shader) {
        if (m_drawer) {
            m_drawer->fullscreen(shader);
        }
    }

    RendererAPI* api() const { return m_api.get(); }

   private:
    RenderGraph& m_graph;
    std::shared_ptr<RendererAPI> m_api;
    IPassDrawer* m_drawer = nullptr;  // null when a graph is built without a renderer (tests)
    std::shared_ptr<Framebuffer> m_target;
};

// One pass contributed to the render graph.
class IRenderPass {
   public:
    virtual ~IRenderPass() = default;

    // Name of the graph node (used for debugging and as the pass's identity in the graph).
    virtual const char* name() const = 0;

    // Declare this pass's resources. Called every time the graph is (re)built: store the returned
    // handles as members and re-declare them here each time, because handles are indices into the
    // current resource table and do not survive a rebuild.
    virtual void setup(RenderGraphBuilder& builder) = 0;

    // Run the pass, resolving the handles stored during setup() through `ctx`.
    virtual void execute(PassContext& ctx) = 0;
};

// A bundle of passes plugged into a renderer from application code via Renderer::addFeature (see
// the worked example at the top of this header). The renderer calls setup() on each pass when it
// builds the frame's graph and execute() when the graph runs. The feature owns its passes.
class RenderFeature {
   public:
    virtual ~RenderFeature() = default;
    virtual const char* name() const = 0;

    // The passes this feature contributes, in declaration order.
    const std::vector<std::unique_ptr<IRenderPass>>& passes() const { return m_passes; }

   protected:
    // Construct and adopt a pass; typically called from the subclass constructor.
    template<typename TPass, typename... Args>
    TPass& addPass(Args&&... args) {
        auto pass = std::make_unique<TPass>(std::forward<Args>(args)...);
        auto& ref = *pass;
        m_passes.push_back(std::move(pass));
        return ref;
    }

    // Adopt an already-constructed pass.
    IRenderPass& adoptPass(std::unique_ptr<IRenderPass> pass) {
        auto& ref = *pass;
        m_passes.push_back(std::move(pass));
        return ref;
    }

   private:
    std::vector<std::unique_ptr<IRenderPass>> m_passes;
};

// A feature that is exactly one pass. Most passes stand alone and need no bundle around them, so
// Renderer::addPass() wraps them in this rather than making callers declare a feature class whose
// only job is to hold one pass. Grouping several passes under a name (shared state, one on/off
// switch) is what RenderFeature is actually for.
class SinglePassFeature : public RenderFeature {
   public:
    explicit SinglePassFeature(std::unique_ptr<IRenderPass> pass) : m_name(pass ? pass->name() : "") {
        if (pass) {
            adoptPass(std::move(pass));
        }
    }
    const char* name() const override { return m_name.c_str(); }

   private:
    std::string m_name;
};

// Add one pass to `graph`: it declares its resources through a builder now, and runs against a
// PassContext when the graph executes. A renderer calls this for every registered pass each time it
// (re)builds the frame's graph -- which is what regenerates the pass's resource handles for the new
// compile. The pass must outlive the graph's execution (the renderer owns both).
inline void addPassToGraph(RenderGraph& graph, IRenderPass& pass, RenderResourceHandle<Framebuffer> scene_color,
                           const std::shared_ptr<RendererAPI>& api, IPassDrawer* drawer = nullptr) {
    auto& graph_pass = graph.addPass(pass.name());
    RenderGraphBuilder builder(graph, graph_pass, scene_color);
    pass.setup(builder);
    const auto target = builder.targetHandle();
    IRenderPass* raw = &pass;
    graph_pass.setExecuteCallback([&graph, raw, api, drawer, target](const RenderGraphPass&) {
        std::shared_ptr<Framebuffer> fb;
        if (target.valid()) {
            if (auto* resource = graph.resourceAt(target.index())) {
                fb = resource->getPhysicalResourceAs<Framebuffer>();
            }
        }
        // Bind the pass's target and size the viewport to it before handing over, so a pass that
        // renders into an off-size target (a 2048x2048 shadow map, a quarter-res bloom buffer)
        // doesn't have to remember to do either.
        if (fb) {
            fb->bind();
            if (api) {
                api->setViewport(0, 0, static_cast<int>(fb->getFormat().width), static_cast<int>(fb->getFormat().height));
            }
        }
        PassContext ctx(graph, api, drawer, fb);
        raw->execute(ctx);
    });
}

// Add every pass a feature contributes, in declaration order.
inline void addFeaturePasses(RenderGraph& graph, const RenderFeature& feature, RenderResourceHandle<Framebuffer> scene_color,
                             const std::shared_ptr<RendererAPI>& api, IPassDrawer* drawer = nullptr) {
    for (const auto& pass : feature.passes()) {
        addPassToGraph(graph, *pass, scene_color, api, drawer);
    }
}
}  // namespace ICE
