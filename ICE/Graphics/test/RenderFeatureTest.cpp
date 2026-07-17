#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "RenderFeature.h"

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
