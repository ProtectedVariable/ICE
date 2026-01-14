//
// Created by Thomas Ibanez on 25.11.20.
//

#pragma once

#include <AssetBank.h>
#include <EngineConfig.h>
#include <GL/gl3w.h>
#include <GraphicsAPI.h>
#include <Project.h>
#include <Registry.h>
#include <RenderSystem.h>
#include <System.h>

#include <vector>

namespace ICE {
class ICEEngine {
   public:
    ICEEngine();

    void initialize(const std::shared_ptr<GraphicsFactory>& graphics_factor, const std::shared_ptr<Window>& window);

    void step();

    void setupScene(const std::shared_ptr<Camera>& camera_);

    std::shared_ptr<Camera> getCamera();

    std::shared_ptr<AssetBank> getAssetBank();

    Entity getSelected() const;

    std::shared_ptr<RendererAPI> getApi() const;

    std::shared_ptr<Project> getProject() const;

    void setProject(const std::shared_ptr<Project>& project);

    void setSelected(Entity selected);

    EngineConfig& getConfig();

    std::shared_ptr<GraphicsFactory> getGraphicsFactory() const;

    std::shared_ptr<Context> getContext() const;

    std::shared_ptr<Framebuffer> getInternalFramebuffer() const;
    void setRenderFramebufferInternal(bool use_internal);

    std::shared_ptr<Window> getWindow() const;

   private:
    std::shared_ptr<GraphicsFactory> m_graphics_factory;
    std::shared_ptr<Context> ctx;
    std::shared_ptr<RendererAPI> api;
    std::shared_ptr<Framebuffer> internalFB;
    std::shared_ptr<Framebuffer> m_target_fb = nullptr;
    std::shared_ptr<Window> m_window;

    std::shared_ptr<Camera> camera;
    std::shared_ptr<Project> project = nullptr;

    std::chrono::steady_clock::time_point lastFrameTime;

    EngineConfig config;
    Registry registry;
};
}  // namespace ICE
