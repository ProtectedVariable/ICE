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
#include "Renderer.h"
#include "RendererConfig.h"
#include "Skybox.h"

namespace ICE {
class ForwardRenderer : public Renderer {
   public:
    ForwardRenderer(const std::shared_ptr<RendererAPI>& api, const std::shared_ptr<Registry>& registry, const std::shared_ptr<AssetBank>& assetBank);

    void submit(Entity e) override;

    void prepareFrame(Camera& camera) override;

    void render() override;

    void endFrame() override;

    void setTarget(const std::shared_ptr<Framebuffer>& fb) override;

    void resize(uint32_t width, uint32_t height) override;

    void setClearColor(Eigen::Vector4f clearColor) override;

   private:
    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<Registry> m_registry;
    std::shared_ptr<AssetBank> m_asset_bank;

    std::shared_ptr<Framebuffer> target = nullptr;
    std::vector<std::function<void(void)>> m_render_commands;
    std::vector<std::pair<RenderComponent, TransformComponent>> m_render_queue;

    RendererConfig config;
    const Skybox* skybox;
};
}  // namespace ICE