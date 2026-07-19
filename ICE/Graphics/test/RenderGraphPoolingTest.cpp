#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "RenderFeature.h"

using namespace ICE;

// Exercises T5's allocation behaviour: the graph compiles once and pools transient resources across
// rebuilds, and it can allocate a Texture2D from a descriptor. A counting factory stands in for the
// backend, so "how many framebuffers did we allocate" is directly observable without GL.

namespace {

class StubFramebuffer : public Framebuffer {
   public:
    explicit StubFramebuffer(const FrameBufferFormat& fmt) : Framebuffer(fmt) {}
    void bind() override { binds++; }
    void unbind() override {}
    void resize(int, int) override {}
    int getTexture() override { return 0; }
    void bindAttachment(int) const override {}
    Eigen::Vector4i readPixel(int, int) override { return Eigen::Vector4i::Zero(); }

    int binds = 0;
};

class StubTexture : public GPUTexture {
   public:
    void bind(uint32_t) const override {}
    int id() const override { return 1; }
};

// Counts what the graph asks the backend to allocate. Everything the graph never calls returns
// null.
class CountingFactory : public GraphicsFactory {
   public:
    std::shared_ptr<Context> createContext(const std::shared_ptr<Window>&) const override { return nullptr; }
    std::shared_ptr<RendererAPI> createRendererAPI() const override { return nullptr; }
    std::shared_ptr<VertexArray> createVertexArray() const override { return nullptr; }
    std::shared_ptr<VertexBuffer> createVertexBuffer() const override { return nullptr; }
    std::shared_ptr<IndexBuffer> createIndexBuffer() const override { return nullptr; }
    std::shared_ptr<UniformBuffer> createUniformBuffer(size_t, size_t) const override { return nullptr; }
    std::shared_ptr<ShaderProgram> createShader(const Shader&) const override { return nullptr; }
    std::shared_ptr<GPUTexture> createTexture2D(const Texture2D&) const override { return nullptr; }
    std::shared_ptr<GPUTexture> createTextureCube(const TextureCube&) const override { return nullptr; }

    std::shared_ptr<Framebuffer> createFramebuffer(const FrameBufferFormat& format) const override {
        framebuffers_created++;
        return std::make_shared<StubFramebuffer>(format);
    }

    std::shared_ptr<GPUTexture> createTexture2D(uint32_t width, uint32_t height, TextureFormat format) const override {
        textures_created++;
        last_texture_width = width;
        last_texture_height = height;
        last_texture_format = format;
        return std::make_shared<StubTexture>();
    }

    mutable int framebuffers_created = 0;
    mutable int textures_created = 0;
    mutable uint32_t last_texture_width = 0;
    mutable uint32_t last_texture_height = 0;
    mutable TextureFormat last_texture_format = TextureFormat::RGBA8;
};

// A pass that creates one off-screen target of the given size and writes it, so the target is what
// the graph binds before execute().
class TargetPass : public IRenderPass {
   public:
    TargetPass(uint32_t size, bool* executed) : m_size(size), m_executed(executed) {}
    const char* name() const override { return "target_pass"; }
    void setup(RenderGraphBuilder& builder) override {
        m_own = builder.create<Framebuffer>({.width = m_size, .height = m_size, .debug_name = "own_target"});
        builder.write(m_own);
        builder.write(builder.sceneColor());  // stay live: contribute to the output
    }
    void execute(PassContext& ctx) override {
        *m_executed = true;
        seen_target = ctx.target();
        resolved_own = ctx.get(m_own);
    }

    std::shared_ptr<Framebuffer> seen_target;
    std::shared_ptr<Framebuffer> resolved_own;

   private:
    uint32_t m_size;
    bool* m_executed;
    RenderResourceHandle<Framebuffer> m_own;
};

// Builds a graph the way ForwardRenderer::rebuildGraph() does, over a counting factory.
struct Frame {
    CountingFactory factory;
    RenderGraph graph{std::shared_ptr<GraphicsFactory>(&factory, [](GraphicsFactory*) {})};
    std::shared_ptr<StubFramebuffer> scene_fb = std::make_shared<StubFramebuffer>(FrameBufferFormat{4, 4, 1});

    void build(IRenderPass* pass) {
        graph.reset();
        auto scene_color = graph.importResource<Framebuffer>("scene_color", scene_fb);
        auto& geometry = graph.addPass("geometry");
        geometry.write("scene_color");
        geometry.setExecuteCallback([](const RenderGraphPass&) {});
        if (pass) {
            addPassToGraph(graph, *pass, scene_color, nullptr);
        }
        graph.setOutput(scene_color);
        graph.compile();
    }
};
}  // namespace

// The headline T5 property: executing many frames off one compile allocates nothing further. This
// is what the old per-frame reset()/compile() broke the moment a pass called create<T>().
TEST(RenderGraphPoolingTest, ExecutingManyFramesAllocatesNothingFurther) {
    bool executed = false;
    TargetPass pass(8, &executed);
    Frame frame;

    frame.build(&pass);
    const int after_compile = frame.factory.framebuffers_created;
    EXPECT_EQ(after_compile, 1);  // the pass's own target (scene_color was imported, not allocated)

    for (int i = 0; i < 60; ++i) {
        frame.graph.execute();
    }

    EXPECT_TRUE(executed);
    EXPECT_EQ(frame.factory.framebuffers_created, after_compile);  // flat across 60 frames
}

// What gets presented comes from the graph's declared output, not from whichever pass the renderer
// assumes produced it. (Today scene_color is the imported geometry framebuffer, so they coincide --
// this pins the resolution path so they can diverge later without silently presenting the wrong
// target.)
TEST(RenderGraphPoolingTest, OutputResolvesToTheDeclaredResource) {
    Frame frame;
    frame.build(nullptr);

    EXPECT_EQ(frame.graph.output<Framebuffer>(), frame.scene_fb);
}

// A graph with no declared output has nothing to present, rather than guessing.
TEST(RenderGraphPoolingTest, OutputIsNullWhenUndeclared) {
    Frame frame;
    frame.graph.reset();
    frame.graph.compile();

    EXPECT_EQ(frame.graph.output<Framebuffer>(), nullptr);
}

// A rebuild whose descriptors are unchanged reuses the pooled resource instead of allocating.
TEST(RenderGraphPoolingTest, RebuildWithSameDescriptorsReusesPooledResource) {
    bool executed = false;
    TargetPass pass(8, &executed);
    Frame frame;

    frame.build(&pass);
    ASSERT_EQ(frame.factory.framebuffers_created, 1);
    frame.graph.execute();
    auto first = pass.resolved_own;

    frame.build(&pass);  // reset() + recompile, same sizes
    frame.graph.execute();

    EXPECT_EQ(frame.factory.framebuffers_created, 1);  // nothing new allocated
    EXPECT_EQ(pass.resolved_own, first);               // literally the same framebuffer back
}

// A resize changes the descriptor, so the pooled resource no longer matches: a new one is
// allocated and the stale size is dropped rather than pinned forever.
TEST(RenderGraphPoolingTest, RebuildWithChangedDescriptorAllocatesAndDropsStale) {
    bool executed = false;
    TargetPass small(8, &executed);
    TargetPass large(16, &executed);
    Frame frame;

    frame.build(&small);
    ASSERT_EQ(frame.factory.framebuffers_created, 1);

    frame.build(&large);  // "resize": different descriptor
    EXPECT_EQ(frame.factory.framebuffers_created, 2);

    // The 8x8 was dropped at the end of the previous compile, so going back allocates afresh
    // rather than resurrecting it -- the pool never grows unboundedly.
    frame.build(&small);
    EXPECT_EQ(frame.factory.framebuffers_created, 3);
}

// The pass's first framebuffer write is bound before execute() and handed back as ctx.target(), so
// a pass body doesn't bind anything itself.
TEST(RenderGraphPoolingTest, PassTargetIsBoundBeforeExecute) {
    bool executed = false;
    TargetPass pass(8, &executed);
    Frame frame;

    frame.build(&pass);
    frame.graph.execute();

    ASSERT_TRUE(pass.seen_target != nullptr);
    EXPECT_EQ(pass.seen_target, pass.resolved_own);  // target() == the framebuffer it created
    auto* own = static_cast<StubFramebuffer*>(pass.seen_target.get());
    EXPECT_GE(own->binds, 1);  // the graph bound it for us
}

// Descriptor-based Texture2D allocation (the "allocateResources only handles RenderTarget" TODO).
TEST(RenderGraphPoolingTest, AllocatesTexture2DFromDescriptor) {
    class TexturePass : public IRenderPass {
       public:
        const char* name() const override { return "texture_pass"; }
        void setup(RenderGraphBuilder& builder) override {
            handle = builder.create<GPUTexture>({.width = 64, .height = 32, .format = TextureFormat::Float16, .debug_name = "ao"});
            builder.write(builder.sceneColor());
        }
        void execute(PassContext& ctx) override { resolved = ctx.get(handle); }
        RenderResourceHandle<GPUTexture> handle;
        std::shared_ptr<GPUTexture> resolved;
    };

    TexturePass pass;
    Frame frame;
    frame.build(&pass);
    frame.graph.execute();

    EXPECT_EQ(frame.factory.textures_created, 1);
    EXPECT_EQ(frame.factory.last_texture_width, 64u);
    EXPECT_EQ(frame.factory.last_texture_height, 32u);
    EXPECT_EQ(frame.factory.last_texture_format, TextureFormat::Float16);
    EXPECT_NE(pass.resolved, nullptr);  // no longer virtual
}
