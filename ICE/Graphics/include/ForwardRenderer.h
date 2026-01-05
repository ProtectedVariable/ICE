//
// Created by Thomas Ibanez on 20.11.20.
//

#pragma once

#include <AssetBank.h>
#include <Entity.h>
#include <GraphicsAPI.h>
#include <Registry.h>

#include <vector>

#include "Camera.h"
#include "Framebuffer.h"
#include "GeometryPass.h"
#include "RenderCommand.h"
#include "Renderer.h"
#include "RendererConfig.h"

namespace ICE {

class ForwardRenderer : public Renderer {
   public:
    ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<GraphicsFactory>& factory);

    void submitSkybox(const Skybox& e) override;
    void submitDrawable(const Drawable& e) override;
    void submitLight(const Light& e) override;

    void prepareFrame(Camera& camera) override;

    std::shared_ptr<Framebuffer> render() override;

    void endFrame() override;

    void resize(uint32_t width, uint32_t height) override;

    void setClearColor(Eigen::Vector4f clearColor) override;
    void setViewport(int x, int y, int w, int h) override;

   private:
    std::shared_ptr<RendererAPI> m_api;
    std::vector<RenderCommand> m_render_commands;

    GeometryPass m_geometry_pass;

    std::shared_ptr<UniformBuffer> m_camera_ubo;
    std::shared_ptr<UniformBuffer> m_light_ubo;

    std::optional<Skybox> m_skybox;
    std::vector<Drawable> m_drawables;
    std::vector<Light> m_lights;

    RendererConfig config;
};
}  // namespace ICE