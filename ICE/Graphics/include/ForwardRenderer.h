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

    std::shared_ptr<Framebuffer> render() override;

    void endFrame() override;

    void present(const std::shared_ptr<Framebuffer> &target, const std::shared_ptr<ShaderProgram> &present_shader) override;

    void resize(uint32_t width, uint32_t height) override;

    void setClearColor(Eigen::Vector4f clearColor) override;
    void setViewport(int x, int y, int w, int h) override;

   private:
    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<GPURegistry> m_gpu_registry;
    std::vector<RenderCommand> m_render_commands;

    GeometryPass m_geometry_pass;

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