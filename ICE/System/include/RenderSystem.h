//
// Created by Thomas Ibanez on 19.11.20.
//
#pragma once

#include <Camera.h>
#include <Framebuffer.h>
#include <LightComponent.h>
#include <RenderComponent.h>
#include <Renderer.h>
#include <SkyboxComponent.h>
#include <System.h>
#include <TransformComponent.h>

namespace ICE {
class Scene;

class RenderSystem : public System {
   public:
    RenderSystem() {};

    void onEntityAdded(Entity e) override;
    void onEntityRemoved(Entity e) override;
    void update(double delta) override;

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
};
}  // namespace ICE
