//
// Created by Thomas Ibanez on 19.11.20.
//
#pragma once

#include <Camera.h>
#include <Framebuffer.h>
#include <GPURegistry.h>
#include <LightComponent.h>
#include <RenderComponent.h>
#include <Renderer.h>
#include <SkyboxComponent.h>
#include <System.h>
#include <TransformComponent.h>

namespace ICE {
class Scene;
class Registry;

class RenderSystem : public System {
   public:
    RenderSystem(const std::shared_ptr<RendererAPI> &api, const std::shared_ptr<GraphicsFactory> &factory, const std::shared_ptr<Registry> &reg,
                 const std::shared_ptr<GPURegistry> &gpu_bank);

    void onEntityAdded(Entity e) override;
    void onEntityRemoved(Entity e) override;
    void update(double delta) override;

    void submitModel(const std::shared_ptr<Model> &model, const Eigen::Matrix4f &transform);

    std::shared_ptr<Renderer> getRenderer() const;
    void setRenderer(const std::shared_ptr<Renderer> &renderer);
    std::shared_ptr<Camera> getCamera() const;
    void setCamera(const std::shared_ptr<Camera> &camera);

    void setTarget(const std::shared_ptr<Framebuffer> &fb);
    void setViewport(int x, int y, int w, int h);

    std::vector<Signature> getSignatures(const ComponentManager &comp_manager) const override {
        Signature signature0;
        signature0.set(comp_manager.getComponentType<RenderComponent>());
        signature0.set(comp_manager.getComponentType<TransformComponent>());
        Signature signature1;
        signature1.set(comp_manager.getComponentType<TransformComponent>());
        signature1.set(comp_manager.getComponentType<LightComponent>());
        Signature signature2;
        signature2.set(comp_manager.getComponentType<SkyboxComponent>());
        return {signature0, signature1, signature2};
    }

   private:
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<Framebuffer> m_target;

    AssetUID m_skybox = NO_ASSET_ID;
    std::vector<Entity> m_render_queue;
    std::vector<Entity> m_lights;

    std::shared_ptr<RendererAPI> m_api;
    std::shared_ptr<GraphicsFactory> m_factory;
    std::shared_ptr<Registry> m_registry;
    std::shared_ptr<GPURegistry> m_gpu_bank;

    std::shared_ptr<VertexArray> m_quad_vao;
};
}  // namespace ICE
