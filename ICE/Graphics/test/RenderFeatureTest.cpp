#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "PerspectiveCamera.h"
#include "Pipeline.h"
#include "PresentPass.h"
#include "RenderFeature.h"
#include "Renderer.h"

using namespace ICE;

// Exercises the T4 public pass/feature seam: typed resource handles, feature registration, and the
// setup()/execute() cycle. Everything here is compile-time graph logic plus a stub framebuffer --
// no GL is invoked, so a null GraphicsFactory and a null RendererAPI are fine.

namespace {

// Stands in for a real framebuffer so an imported resource has something to resolve to.
class StubFramebuffer : public Framebuffer {
   public:
    StubFramebuffer() : Framebuffer({4, 4, 1}) {}
    void bind() override {}
    void unbind() override {}
    void resize(int, int) override {}
    int getTexture() override { return 0; }
    void bindAttachment(int) const override {}
    Eigen::Vector4i readPixel(int, int) override { return Eigen::Vector4i::Zero(); }
};

// What a pass observed, so a test can assert on it after the graph ran.
struct Trace {
    std::vector<std::string>* order = nullptr;
    int setups = 0;
    int executes = 0;
    std::shared_ptr<Framebuffer> resolved_scene_color;
    bool own_target_was_virtual = false;
};

// --- Application code: an outline feature, written entirely against the public seam ------------
// This is the acceptance case -- it adds a pass to the frame without a single engine edit. It reads
// the engine's scene colour, declares its own target, and writes the scene colour back so it
// contributes to the output (and is therefore not culled).
class OutlinePass : public IRenderPass {
   public:
    explicit OutlinePass(Trace* trace) : m_trace(trace) {}

    const char* name() const override { return "outline"; }

    void setup(RenderGraphBuilder& builder) override {
        m_trace->setups++;
        // Handles are re-declared on every rebuild; these overwrite last compile's.
        m_scene_color = builder.read(builder.sceneColor());
        m_target = builder.create<Framebuffer>({.width = 8, .height = 8, .debug_name = "outline_target"});
        builder.write(builder.sceneColor());
    }

    void execute(PassContext& ctx) override {
        m_trace->executes++;
        m_trace->order->push_back("outline");
        m_trace->resolved_scene_color = ctx.get(m_scene_color);
        m_trace->own_target_was_virtual = (ctx.get(m_target) == nullptr);
    }

   private:
    Trace* m_trace;
    RenderResourceHandle<Framebuffer> m_scene_color;
    RenderResourceHandle<Framebuffer> m_target;
};

class OutlineFeature : public RenderFeature {
   public:
    explicit OutlineFeature(Trace* trace) { addPass<OutlinePass>(trace); }
    const char* name() const override { return "outline"; }
};

// A pass that writes only its own resource: nothing reads it and it isn't the output, so the graph
// culls it.
class DeadEndPass : public IRenderPass {
   public:
    explicit DeadEndPass(Trace* trace) : m_trace(trace) {}
    const char* name() const override { return "dead_end"; }
    void setup(RenderGraphBuilder& builder) override {
        auto own = builder.create<Framebuffer>({.width = 4, .height = 4, .debug_name = "nobody_reads_this"});
        builder.write(own);
    }
    void execute(PassContext&) override {
        m_trace->executes++;
        m_trace->order->push_back("dead_end");
    }

   private:
    Trace* m_trace;
};

class DeadEndFeature : public RenderFeature {
   public:
    explicit DeadEndFeature(Trace* trace) { addPass<DeadEndPass>(trace); }
    const char* name() const override { return "dead_end"; }
};

// A two-pass feature: the first renders into an intermediate target, the second consumes it and
// writes the scene colour. The second pass reads the handle the first published during its own
// setup() -- passes are set up in declaration order, so that handle is from the current compile.
//
// Note the topology: only the last pass writes scene_color and no earlier pass reads it. Two passes
// that both read *and* write the same resource would make each depend on the other (this graph has
// no resource versioning), which compile() reports as a cycle.
class ChainFirstPass : public IRenderPass {
   public:
    explicit ChainFirstPass(Trace* trace) : m_trace(trace) {}
    const char* name() const override { return "chain_first"; }
    void setup(RenderGraphBuilder& builder) override {
        intermediate = builder.create<Framebuffer>({.width = 8, .height = 8, .debug_name = "chain_mid"});
        builder.write(intermediate);
    }
    void execute(PassContext&) override {
        m_trace->executes++;
        m_trace->order->push_back("chain_first");
    }

    RenderResourceHandle<Framebuffer> intermediate;  // published for the next pass

   private:
    Trace* m_trace;
};

class ChainSecondPass : public IRenderPass {
   public:
    ChainSecondPass(Trace* trace, ChainFirstPass* first) : m_trace(trace), m_first(first) {}
    const char* name() const override { return "chain_second"; }
    void setup(RenderGraphBuilder& builder) override {
        builder.read(m_first->intermediate);
        builder.write(builder.sceneColor());
    }
    void execute(PassContext&) override {
        m_trace->executes++;
        m_trace->order->push_back("chain_second");
    }

   private:
    Trace* m_trace;
    ChainFirstPass* m_first;
};

class ChainFeature : public RenderFeature {
   public:
    explicit ChainFeature(Trace* trace) {
        auto& first = addPass<ChainFirstPass>(trace);
        addPass<ChainSecondPass>(trace, &first);
    }
    const char* name() const override { return "chain"; }
};

// Mirrors what ForwardRenderer::render() does around the feature seam, minus the GL: import the
// scene colour, add the engine's geometry pass through the (internal) string layer, then let each
// feature contribute its passes via the same addFeaturePasses() the renderer uses.
struct Frame {
    RenderGraph graph{nullptr};
    std::shared_ptr<StubFramebuffer> scene_fb = std::make_shared<StubFramebuffer>();
    std::vector<std::string> order;

    RenderResourceHandle<Framebuffer> build(const std::vector<RenderFeature*>& features) {
        graph.reset();
        auto scene_color = graph.importResource<Framebuffer>("scene_color", scene_fb);
        auto& geometry = graph.addPass("geometry");
        geometry.write("scene_color");
        geometry.setExecuteCallback([this](const RenderGraphPass&) { order.push_back("geometry"); });
        for (auto* feature : features) {
            addFeaturePasses(graph, *feature, scene_color, nullptr);
        }
        graph.setOutput(scene_color);
        graph.compile();
        return scene_color;
    }
};
}  // namespace

// The acceptance case: a feature written as application code contributes a pass to the frame, runs
// after the pass that produces what it reads, and resolves its handles at execute time.
TEST(RenderFeatureTest, RegisteredFeatureAddsPassToTheFrame) {
    Trace trace;
    Frame frame;
    trace.order = &frame.order;
    OutlineFeature feature(&trace);

    frame.build({&feature});
    frame.graph.execute();

    EXPECT_EQ(trace.setups, 1);
    EXPECT_EQ(trace.executes, 1);
    ASSERT_EQ(frame.order.size(), 2u);
    EXPECT_EQ(frame.order[0], "geometry");  // outline reads scene_color, which geometry writes
    EXPECT_EQ(frame.order[1], "outline");
}

// A handle resolves to the physical resource the engine imported -- this is what replaces
// getResource<T>("scene_color") returning null on a typo.
TEST(RenderFeatureTest, HandleResolvesToImportedResource) {
    Trace trace;
    Frame frame;
    trace.order = &frame.order;
    OutlineFeature feature(&trace);

    frame.build({&feature});
    frame.graph.execute();

    EXPECT_EQ(trace.resolved_scene_color, frame.scene_fb);
    // The pass's own created RenderTarget stays virtual here only because this graph has a null
    // factory; with a real factory compile() allocates it.
    EXPECT_TRUE(trace.own_target_was_virtual);
}

// Handles are indices into the current resource table: a rebuild re-runs setup(), so a feature's
// handles are regenerated and stay valid rather than dangling across reset().
TEST(RenderFeatureTest, HandlesAreRegeneratedOnRebuild) {
    Trace trace;
    Frame frame;
    trace.order = &frame.order;
    OutlineFeature feature(&trace);

    frame.build({&feature});
    frame.graph.execute();
    frame.order.clear();

    frame.build({&feature});  // reset() + re-setup, as a resize would do
    frame.graph.execute();

    EXPECT_EQ(trace.setups, 2);
    EXPECT_EQ(trace.executes, 2);
    EXPECT_EQ(trace.resolved_scene_color, frame.scene_fb);  // still resolves after the rebuild
    ASSERT_EQ(frame.order.size(), 2u);
    EXPECT_EQ(frame.order[1], "outline");
}

// A feature pass that doesn't contribute to the output is culled -- the documented reason a
// post-process pass must write sceneColor().
TEST(RenderFeatureTest, FeaturePassNotContributingToOutputIsCulled) {
    Trace trace;
    Frame frame;
    trace.order = &frame.order;
    DeadEndFeature feature(&trace);

    frame.build({&feature});
    frame.graph.execute();

    EXPECT_EQ(trace.executes, 0);
    EXPECT_EQ(std::find(frame.order.begin(), frame.order.end(), "dead_end"), frame.order.end());
}

// A feature can contribute several passes; the graph orders them by the resources they exchange,
// and a pass can consume a handle a previous pass of the same feature declared.
TEST(RenderFeatureTest, FeatureCanContributeMultiplePassesInOrder) {
    Trace trace;
    Frame frame;
    trace.order = &frame.order;
    ChainFeature feature(&trace);

    frame.build({&feature});
    frame.graph.execute();

    EXPECT_EQ(trace.executes, 2);
    const auto first = std::find(frame.order.begin(), frame.order.end(), "chain_first");
    const auto second = std::find(frame.order.begin(), frame.order.end(), "chain_second");
    ASSERT_NE(first, frame.order.end());
    ASSERT_NE(second, frame.order.end());
    EXPECT_LT(first, second);  // chain_second reads what chain_first writes
}

// The common path: a lone pass needs no feature class around it. Renderer::addPass() wraps it in a
// SinglePassFeature, which behaves exactly like a hand-written one-pass feature.
TEST(RenderFeatureTest, SinglePassFeatureWrapsALonePass) {
    Trace trace;
    Frame frame;
    trace.order = &frame.order;
    SinglePassFeature feature(std::make_unique<OutlinePass>(&trace));

    EXPECT_STREQ(feature.name(), "outline");  // takes the pass's own name
    ASSERT_EQ(feature.passes().size(), 1u);

    frame.build({&feature});
    frame.graph.execute();

    EXPECT_EQ(trace.setups, 1);
    EXPECT_EQ(trace.executes, 1);
    ASSERT_EQ(frame.order.size(), 2u);
    EXPECT_EQ(frame.order[1], "outline");
}

// An undeclared (default-constructed) handle is the one hole the type system leaves: it throws
// rather than silently resolving to null at draw time.
TEST(RenderFeatureTest, UndeclaredHandleThrows) {
    RenderGraph graph(nullptr);
    auto& pass = graph.addPass("p");
    RenderGraphBuilder builder(graph, pass, RenderResourceHandle<Framebuffer>{});

    RenderResourceHandle<Framebuffer> undeclared;
    EXPECT_FALSE(undeclared.valid());
    EXPECT_THROW(builder.read(undeclared), std::runtime_error);
}

namespace {
// Records what it saw through ctx.frame(), so a test can assert the conduit delivered it.
class FrameProbePass : public IRenderPass {
   public:
    const char* name() const override { return "frame_probe"; }
    void setup(RenderGraphBuilder& b) override { b.write(b.sceneColor()); }
    void execute(PassContext& ctx) override {
        saw_frame = ctx.hasFrame();
        if (saw_frame) {
            camera = ctx.frame().camera;
            commands = ctx.frame().commands;
        }
    }
    bool saw_frame = false;
    Camera* camera = nullptr;
    const std::vector<RenderCommand>* commands = nullptr;
};
}  // namespace

// Phase 1 of the scriptable-pipeline migration: the FrameContext the renderer injects reaches a pass
// through PassContext::frame(), carrying this frame's data. (The pointers are sentinels -- only
// compared, never dereferenced -- so no GL or real camera is needed.)
TEST(RenderFeatureTest, FrameContextReachesPassThroughContext) {
    RenderGraph graph(nullptr);
    auto scene_fb = std::make_shared<StubFramebuffer>();
    auto scene_color = graph.importResource<Framebuffer>("scene_color", scene_fb);

    std::vector<RenderCommand> commands;
    FrameContext frame;
    frame.camera = reinterpret_cast<Camera*>(0xC0FFEE);
    frame.commands = &commands;

    FrameProbePass probe;
    addPassToGraph(graph, probe, scene_color, /*api=*/nullptr, &frame);
    graph.setOutput(scene_color);
    graph.compile();
    graph.execute();

    EXPECT_TRUE(probe.saw_frame);
    EXPECT_EQ(probe.camera, reinterpret_cast<Camera*>(0xC0FFEE));
    EXPECT_EQ(probe.commands, &commands);
}

// A graph built without a renderer (the graph-logic tests) has no frame: hasFrame() is false rather
// than frame() dereferencing null.
TEST(RenderFeatureTest, NoFrameContextIsSafe) {
    RenderGraph graph(nullptr);
    auto scene_fb = std::make_shared<StubFramebuffer>();
    auto scene_color = graph.importResource<Framebuffer>("scene_color", scene_fb);

    FrameProbePass probe;
    probe.saw_frame = true;  // ensure execute() actually flips it to false
    addPassToGraph(graph, probe, scene_color, /*api=*/nullptr);  // no frame threaded
    graph.setOutput(scene_color);
    graph.compile();
    graph.execute();

    EXPECT_FALSE(probe.saw_frame);
}

// Phase 2: PresentPass declares the right resources in setup() -- it reads the scene colour and
// writes the backbuffer output. (Structure only; execute() does GL and is not run here.)
TEST(RenderFeatureTest, PresentPassDeclaresSceneColorAndBackbuffer) {
    RenderGraph graph(nullptr);
    auto scene_fb = std::make_shared<StubFramebuffer>();
    auto scene_color = graph.importResource<Framebuffer>("scene_color", scene_fb);

    auto& node = graph.addPass("present");
    RenderGraphBuilder builder(graph, node, scene_color);
    PresentPass present;
    present.setup(builder);

    const auto& reads = node.getReads();
    const auto& writes = node.getWrites();
    EXPECT_NE(std::find(reads.begin(), reads.end(), "scene_color"), reads.end());
    EXPECT_NE(std::find(writes.begin(), writes.end(), "backbuffer"), writes.end());
    // It declares no framebuffer *handle* write, so the graph binds it no target -- it binds the
    // real backbuffer itself in execute().
    EXPECT_FALSE(builder.targetHandle().valid());
}

// presentsToBackbuffer() makes a pass the graph's output, so it -- and the scene pass it reads --
// survive culling and run.
TEST(RenderFeatureTest, PresentsToBackbufferSurvivesCulling) {
    struct Presenter : public IRenderPass {
        explicit Presenter(bool* ran) : m_ran(ran) {}
        const char* name() const override { return "presenter"; }
        void setup(RenderGraphBuilder& b) override {
            b.read(b.sceneColor());
            b.presentsToBackbuffer();
        }
        void execute(PassContext&) override { *m_ran = true; }
        bool* m_ran;
    };

    RenderGraph graph(nullptr);
    auto scene_fb = std::make_shared<StubFramebuffer>();
    auto scene_color = graph.importResource<Framebuffer>("scene_color", scene_fb);

    bool scene_ran = false;
    auto& scene = graph.addPass("scene");
    scene.write("scene_color");
    scene.setExecuteCallback([&](const RenderGraphPass&) { scene_ran = true; });

    bool present_ran = false;
    Presenter presenter(&present_ran);
    addPassToGraph(graph, presenter, scene_color, nullptr);

    graph.compile();
    graph.execute();

    EXPECT_TRUE(present_ran);  // not culled: presentsToBackbuffer() made it the output
    EXPECT_TRUE(scene_ran);    // its input producer is kept too
}

namespace {
// GL-free recording passes mirroring ForwardPipeline's built-ins, for a Pipeline structure test.
class RecGeometry : public IRenderPass {
   public:
    explicit RecGeometry(std::vector<std::string>* order) : m_order(order) {}
    const char* name() const override { return "geometry"; }
    void setup(RenderGraphBuilder& b) override {
        m_color = b.create<Framebuffer>({.width = 4, .height = 4, .debug_name = "scene_color"});
        b.write(m_color);
    }
    void execute(PassContext&) override { m_order->push_back("geometry"); }
    RenderResourceHandle<Framebuffer> color() const { return m_color; }

   private:
    std::vector<std::string>* m_order;
    RenderResourceHandle<Framebuffer> m_color;
};

class RecFeaturePass : public IRenderPass {
   public:
    explicit RecFeaturePass(std::vector<std::string>* order) : m_order(order) {}
    const char* name() const override { return "feature"; }
    void setup(RenderGraphBuilder& b) override {
        b.read(b.sceneColor());
        b.write(b.sceneColor());  // write so present is ordered after it
    }
    void execute(PassContext&) override { m_order->push_back("feature"); }

   private:
    std::vector<std::string>* m_order;
};

class RecFeature : public RenderFeature {
   public:
    explicit RecFeature(std::vector<std::string>* order) { addPass<RecFeaturePass>(order); }
    const char* name() const override { return "rec"; }
};

class RecPresent : public IRenderPass {
   public:
    explicit RecPresent(std::vector<std::string>* order) : m_order(order) {}
    const char* name() const override { return "present"; }
    void setup(RenderGraphBuilder& b) override {
        b.read(b.sceneColor());
        b.presentsToBackbuffer();
    }
    void execute(PassContext&) override { m_order->push_back("present"); }

   private:
    std::vector<std::string>* m_order;
};

// A pipeline mirroring ForwardPipeline's assembly (geometry -> features -> present) with the passes
// above -- proving the Pipeline abstraction assembles and orders a frame with no privileged passes.
class RecPipeline : public Pipeline {
   public:
    explicit RecPipeline(std::vector<std::string>* order) : m_geo(order), m_present(order) {}
    void build(RenderGraph& graph, const PipelineContext& ctx) override {
        addPassToGraph(graph, m_geo, {}, ctx.api, ctx.frame);
        const auto scene_color = m_geo.color();
        if (ctx.features) {
            for (const auto& feature : *ctx.features) {
                addFeaturePasses(graph, *feature, scene_color, ctx.api, ctx.frame);
            }
        }
        addPassToGraph(graph, m_present, scene_color, ctx.api, ctx.frame);
    }

    RecGeometry m_geo;
    RecPresent m_present;
};
}  // namespace

// Phase 4: a pipeline assembles the whole frame -- geometry creates the scene colour, features run
// over it, present composites -- with no privileged passes, and the graph orders them by the
// resources they exchange. (GL-free: a null factory leaves targets virtual; passes just record.)
TEST(RenderFeatureTest, PipelineAssemblesGeometryFeaturesPresentInOrder) {
    std::vector<std::string> order;
    std::vector<std::unique_ptr<RenderFeature>> features;
    features.push_back(std::make_unique<RecFeature>(&order));

    RecPipeline pipeline(&order);
    RenderGraph graph(nullptr);
    PipelineContext ctx;
    ctx.api = nullptr;
    ctx.frame = nullptr;
    ctx.render_width = 4;
    ctx.render_height = 4;
    ctx.features = &features;

    pipeline.build(graph, ctx);
    graph.compile();
    graph.execute();

    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], "geometry");  // creates scene_color
    EXPECT_EQ(order[1], "feature");   // reads+writes it
    EXPECT_EQ(order[2], "present");   // reads it, is the output
}

namespace {
// Records the primitive calls the default Renderer::drawFrame() makes, in order.
class CallRecordingRenderer : public Renderer {
   public:
    std::vector<std::string> calls;
    int drawables = 0, lights = 0;

    void submitSkybox(const Skybox&) override { calls.push_back("skybox"); }
    void submitDrawable(Drawable) override {
        calls.push_back("drawable");
        drawables++;
    }
    void submitLight(const Light&) override {
        calls.push_back("light");
        lights++;
    }
    void setPresentTarget(const std::shared_ptr<Framebuffer>&) override { calls.push_back("target"); }
    void setPresentShader(const std::shared_ptr<ShaderProgram>&) override { calls.push_back("shader"); }
    void prepareFrame(Camera&) override { calls.push_back("prepare"); }
    void render() override { calls.push_back("render"); }
    void endFrame() override { calls.push_back("end"); }
    void resize(uint32_t, uint32_t) override {}
    void setClearColor(Eigen::Vector4f) override {}
    void setViewport(int, int, int, int) override {}
};
}  // namespace

// Phase 5: the default drawFrame() orchestrates the frame through the primitives -- configure
// present, submit the visible set, then prepare/render/end -- so the RenderSystem makes one call.
TEST(RenderFeatureTest, DrawFrameOrchestratesThePrimitives) {
    PerspectiveCamera camera(60.0, 16.0 / 9.0, 0.01, 100.0);
    FrameInputs inputs;
    inputs.camera = &camera;
    inputs.skybox = Skybox{};
    inputs.drawables.resize(2);  // two visible drawables
    inputs.lights.resize(3);

    CallRecordingRenderer renderer;
    renderer.drawFrame(std::move(inputs));

    EXPECT_EQ(renderer.drawables, 2);
    EXPECT_EQ(renderer.lights, 3);
    // present is configured, the visible set submitted, then prepare -> render -> end.
    const std::vector<std::string> expected = {"target", "shader", "skybox", "drawable", "drawable",
                                               "light",  "light",  "light",  "prepare",  "render", "end"};
    EXPECT_EQ(renderer.calls, expected);
}

namespace {
// A pipeline-owned overlay pass (not an app feature): reads + writes the scene colour.
class OverlayPass : public IRenderPass {
   public:
    explicit OverlayPass(std::vector<std::string>* order) : m_order(order) {}
    const char* name() const override { return "overlay"; }
    void setup(RenderGraphBuilder& b) override {
        b.read(b.sceneColor());
        b.write(b.sceneColor());
    }
    void execute(PassContext&) override { m_order->push_back("overlay"); }

   private:
    std::vector<std::string>* m_order;
};

// A custom pipeline: geometry -> overlay -> present, with the overlay hardcoded in the pipeline (no
// app features at all). Proves a pipeline defines its own structure and that geometry/present are
// not privileged -- a Phase 6 replacement is free to restructure the frame.
class OverlayPipeline : public Pipeline {
   public:
    explicit OverlayPipeline(std::vector<std::string>* order) : m_geo(order), m_overlay(order), m_present(order) {}
    void build(RenderGraph& graph, const PipelineContext& ctx) override {
        addPassToGraph(graph, m_geo, {}, ctx.api, ctx.frame);
        const auto scene_color = m_geo.color();
        addPassToGraph(graph, m_overlay, scene_color, ctx.api, ctx.frame);
        addPassToGraph(graph, m_present, scene_color, ctx.api, ctx.frame);
    }

    RecGeometry m_geo;
    OverlayPass m_overlay;
    RecPresent m_present;
};
}  // namespace

// Phase 6: a custom pipeline restructures the frame -- here inserting a pipeline-owned overlay pass
// between geometry and present, with no app features. This is what setPipeline() installs; the graph
// orders the passes by the scene colour they exchange.
TEST(RenderFeatureTest, CustomPipelineDefinesItsOwnStructure) {
    std::vector<std::string> order;
    OverlayPipeline pipeline(&order);

    RenderGraph graph(nullptr);
    PipelineContext ctx;
    ctx.api = nullptr;
    ctx.frame = nullptr;
    ctx.render_width = 4;
    ctx.render_height = 4;
    ctx.features = nullptr;

    pipeline.build(graph, ctx);
    graph.compile();
    graph.execute();

    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], "geometry");
    EXPECT_EQ(order[1], "overlay");  // the pipeline's own pass, between geometry and present
    EXPECT_EQ(order[2], "present");
}
