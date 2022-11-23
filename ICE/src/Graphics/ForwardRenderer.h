//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_FORWARDRENDERER_H
#define ICE_FORWARDRENDERER_H

#include "Renderer.h"
#include <Scene/Entity.h>
#include "RendererConfig.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Skybox.h"
#include <vector>

namespace ICE {
    class ForwardRenderer : public Renderer {
    public:
        void initialize(RendererConfig config, AssetBank* assetBank) override;

        void submitScene(Scene *scene) override;

        void submit(Entity e) override;

        void prepareFrame(Camera& camera) override;

        void render() override;

        void endFrame() override;

        void setTarget(Framebuffer *target) override;

        void resize(uint32_t width, uint32_t height) override;

        void setClearColor(Eigen::Vector4f clearColor) override;
    private:
        std::vector<Entity*> renderableEntities;
        std::vector<Entity*> lightEntities;
        RendererAPI* api;
        RendererConfig config;
        Framebuffer* target;
        Skybox* skybox;
        AssetBank* assetBank;
    };
}


#endif //ICE_FORWARDRENDERER_H
