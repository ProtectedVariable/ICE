//
// Created by Thomas Ibanez on 20.11.20.
//

#pragma once

#include <AssetBank.h>
#include <Entity.h>
#include <GPURegistry.h>
#include <GraphicsAPI.h>

#include <map>
#include <optional>
#include <tuple>
#include <vector>

#include "Camera.h"
#include "ForwardPipeline.h"
#include "FrameContext.h"
#include "Framebuffer.h"
#include "InstanceData.h"
#include "RenderCommand.h"
#include "RenderGraph.h"
#include "Renderer.h"
#include "RendererConfig.h"

namespace ICE {

class ForwardRenderer : public Renderer {
   public:
    ForwardRenderer(const std::shared_ptr<RendererAPI> &api, const std::shared_ptr<GraphicsFactory> &factory,
                    const std::shared_ptr<GPURegistry> &gpu_registry);

    void submitSkybox(const Skybox &e) override;
    void submitDrawable(Drawable e) override;
    void submitLight(const Light &e) override;

    void prepareFrame(Camera &camera) override;

    void render() override;

    void endFrame() override;

    void setPresentTarget(const std::shared_ptr<Framebuffer> &target) override { m_present_target = target; }
    void setPresentShader(const std::shared_ptr<ShaderProgram> &present_shader) override { m_present_shader = present_shader; }

    void resize(uint32_t width, uint32_t height) override;

    void setClearColor(Eigen::Vector4f clearColor) override;
    void setViewport(int x, int y, int w, int h) override;

    void addPass(std::unique_ptr<IRenderPass> pass) override;
    void addFeature(std::unique_ptr<RenderFeature> feature) override;

    void setPipeline(std::unique_ptr<Pipeline> pipeline) override {
        if (pipeline) {
            m_pipeline = std::move(pipeline);
            m_graph_dirty = true;  // rebuild the graph with the new pipeline next frame
        }
    }

   private:
    // Tear down and rebuild the frame's graph by handing it to the pipeline (which declares the
    // passes), then compile. Only called when m_graph_dirty (see render()).
    void rebuildGraph();

    // Upload `camera` into the shared camera UBO (what the geometry shaders read).
    void uploadCameraUBO(Camera& camera);

    // Refresh the per-frame conduit handed to passes (see FrameContext). Called each frame before
    // the graph executes; its address is stable so compiled pass callbacks read fresh data.
    void updateFrameContext();
    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<GraphicsFactory> m_factory;
    std::shared_ptr<GPURegistry> m_gpu_registry;
    std::vector<RenderCommand> m_render_commands;

    // Per-frame data + services passed to render passes (Phase 1 of the scriptable-pipeline
    // migration). Populated but not yet consumed by any shipped pass.
    FrameContext m_frame_context;

    // The pipeline that assembles the frame's graph (Phase 4). Owns the built-in geometry/present
    // passes; defaults to ForwardPipeline. Replaceable (Phase 6) to define a custom frame.
    std::unique_ptr<Pipeline> m_pipeline;
    RenderGraph m_graph;

    // The graph is compiled once and re-executed each frame; this marks it for rebuild when
    // something that changes its shape happens -- a resize (resource descriptors change) or a
    // newly registered pass/feature. Rebuilding is what regenerates passes' resource handles.
    bool m_graph_dirty = true;

    // The frame's present target + shader, set per frame by the RenderSystem and read by the
    // graph's present pass. Target null = the window's default framebuffer; a framebuffer = the
    // editor's render-to-texture viewport.
    std::shared_ptr<Framebuffer> m_present_target;
    std::shared_ptr<ShaderProgram> m_present_shader;

    // The camera the current frame was prepared with (see prepareFrame), handed to passes through
    // the frame context.
    Camera* m_frame_camera = nullptr;

    // The current render size (set by resize, from the viewport). The geometry pass creates the
    // scene-colour target at this size each rebuild.
    uint32_t m_render_width = 1;
    uint32_t m_render_height = 1;

    // Application-registered features, in registration order. Their passes are added to the graph
    // each time it is rebuilt (see rebuildGraph()).
    std::vector<std::unique_ptr<RenderFeature>> m_features;

    // The full-screen quad the present/post passes draw, shared through the frame context.
    std::shared_ptr<VertexArray> m_present_quad;

    std::shared_ptr<UniformBuffer> m_camera_ubo;
    std::shared_ptr<UniformBuffer> m_light_ubo;

    std::optional<Skybox> m_skybox;
    std::vector<Drawable> m_drawables;
    std::vector<Light> m_lights;

    // Instance batching storage, keyed by the exact (mesh, material, shader) triple so
    // distinct batches can never collide into one (the old XOR-hashed uint64 key could).
    using BatchKey = std::tuple<GPUMesh*, Material*, ShaderProgram*>;
    std::map<BatchKey, std::vector<InstanceData>> m_instance_batches;

    RendererConfig config;
};
}  // namespace ICE