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
#include "Framebuffer.h"
#include "GeometryPass.h"
#include "InstanceData.h"
#include "RenderCommand.h"
#include "RenderGraph.h"
#include "Renderer.h"
#include "RendererConfig.h"

namespace ICE {

// IPassDrawer is implemented here (not on Renderer) because it is the pass-facing half of the
// renderer: what a graph pass may ask it to draw, handed out through PassContext.
class ForwardRenderer : public Renderer, public IPassDrawer {
   public:
    ForwardRenderer(const std::shared_ptr<RendererAPI> &api, const std::shared_ptr<GraphicsFactory> &factory,
                    const std::shared_ptr<GPURegistry> &gpu_registry);

    void submitSkybox(const Skybox &e) override;
    void submitDrawable(Drawable e) override;
    void submitLight(const Light &e) override;

    void prepareFrame(Camera &camera) override;

    std::shared_ptr<Framebuffer> render() override;

    void endFrame() override;

    void setPresentTarget(const std::shared_ptr<Framebuffer> &target) override { m_present_target = target; }
    void setPresentShader(const std::shared_ptr<ShaderProgram> &present_shader) override { m_present_shader = present_shader; }

    void resize(uint32_t width, uint32_t height) override;

    void setClearColor(Eigen::Vector4f clearColor) override;
    void setViewport(int x, int y, int w, int h) override;

    void addPass(std::unique_ptr<IRenderPass> pass) override;
    void addFeature(std::unique_ptr<RenderFeature> feature) override;

    // --- IPassDrawer (what a graph pass can ask the renderer to draw) ---------------------------
    void drawScene(Camera& camera, ShaderProgram* override_shader) override;
    void fullscreen(ShaderProgram* shader) override;

   private:
    // Tear down and rebuild the frame's graph: import scene_color, add the geometry pass, let every
    // registered feature contribute, then compile. Only called when m_graph_dirty (see render()).
    void rebuildGraph();

    // Upload `camera` into the shared camera UBO (what the geometry shaders read).
    void uploadCameraUBO(Camera& camera);
    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<GPURegistry> m_gpu_registry;
    std::vector<RenderCommand> m_render_commands;

    GeometryPass m_geometry_pass;
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

    // The camera the current frame was prepared with (see prepareFrame). Borrowed for the frame
    // only: drawScene() restores the camera UBO to it so a pass drawing from another point of view
    // (a shadow pass) can't leak that view into later passes.
    Camera* m_frame_camera = nullptr;

    // Application-registered features, in registration order. Their passes are added to the graph
    // each time it is rebuilt (see rebuildGraph()).
    std::vector<std::unique_ptr<RenderFeature>> m_features;

    // Owned by the renderer for the present pass: the full-screen quad the final blit draws, and
    // the most recent render() result it composites from.
    std::shared_ptr<VertexArray> m_present_quad;
    std::shared_ptr<Framebuffer> m_output_fb;

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