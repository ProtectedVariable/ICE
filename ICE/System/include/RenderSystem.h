//
// Created by Thomas Ibanez on 19.11.20.
//
#pragma once

#include <Camera.h>
#include <Framebuffer.h>
#include <GPURegistry.h>
#include <JobScheduler.h>
#include <LightComponent.h>
#include <RenderComponent.h>
#include <Renderer.h>
#include <SkyboxComponent.h>
#include <System.h>
#include <TransformComponent.h>

namespace ICE {
class Scene;
class Registry;
class Model;  // only referenced as shared_ptr in a declaration; full type not needed here

struct CullingData {
    uint32_t lastTransformVersion = 0xFFFFFFFF;
    AssetUID lastMesh = 0;

    Eigen::Vector3f worldCenter;
    Eigen::Vector3f worldExtents;
};

class RenderSystem : public System {
   public:
    // The render system no longer touches the graphics API directly: it only culls, submits the
    // visible set to the renderer, and asks the renderer to present. It therefore needs the
    // registry (to read components) and the GPU bank (to resolve mesh/material/shader handles).
    RenderSystem(const std::shared_ptr<Registry> &reg, const std::shared_ptr<GPURegistry> &gpu_bank);

    void onEntityAdded(Entity e) override;
    void onEntityRemoved(Entity e) override;
    void update(double delta) override;

    int updateOrder() const override { return RenderSystemOrder; }

    void submitModel(const std::shared_ptr<Model> &model, const Eigen::Matrix4f &transform);

    std::shared_ptr<Renderer> getRenderer() const;
    void setRenderer(const std::shared_ptr<Renderer> &renderer);
    std::shared_ptr<Camera> getCamera() const;
    void setCamera(const std::shared_ptr<Camera> &camera);

    void setTarget(const std::shared_ptr<Framebuffer> &fb);
    void setViewport(int x, int y, int w, int h);

    // Opt-in parallel cull+build. With a scheduler set, the per-entity frustum culling, skinning
    // and drawable assembly run across worker threads (all GL/asset access stays on this thread).
    // Null (the default) keeps the single-threaded path. See update().
    void setScheduler(const std::shared_ptr<JobScheduler> &scheduler) { m_scheduler = scheduler; }

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

    // Non-owning back-reference: the Registry owns this system, so a shared_ptr here formed
    // a Registry -> SystemManager -> this -> Registry cycle. The Registry outlives its systems.
    Registry* m_registry = nullptr;
    std::shared_ptr<GPURegistry> m_gpu_bank;

    // Full-screen present shader, resolved from the GPU bank once and reused, instead of a
    // per-frame string lookup. The renderer's present pass consumes it.
    std::shared_ptr<ShaderProgram> m_lastpass_shader;

    // Optional work-stealing scheduler for the parallel cull+build path (null => single-threaded).
    std::shared_ptr<JobScheduler> m_scheduler;

    std::unordered_map<Entity, CullingData> m_culling_cache;
};
}  // namespace ICE
