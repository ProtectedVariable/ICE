//
// Created by Thomas Ibanez on 19.11.20.
//
#pragma once

#include <Camera.h>
#include <Framebuffer.h>
#include <Renderer.h>
#include <System.h>

namespace ICE {
class Scene;

class RenderSystem : public System {
   public:
    RenderSystem(){};

    void onEntityAdded(Entity e) override;
    void onEntityRemoved(Entity e) override;
    void update(double delta) override;

    std::shared_ptr<Renderer> getRenderer() const;
    void setRenderer(const std::shared_ptr<Renderer> &renderer);
    std::shared_ptr<Camera> getCamera() const;
    void setCamera(const std::shared_ptr<Camera> &camera);

    void setTarget(const std::shared_ptr<Framebuffer> &fb);
    void setViewport(int x, int y, int w, int h);

   private:
    std::shared_ptr<Renderer> m_renderer;
    std::shared_ptr<Camera> m_camera;
};
}  // namespace ICE
