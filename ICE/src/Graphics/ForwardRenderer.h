//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_FORWARDRENDERER_H
#define ICE_FORWARDRENDERER_H

#include "Renderer.h"
#include "RendererConfig.h"
#include "Camera.h"
#include "FrameBuffer.h"

namespace ICE {
    class ForwardRenderer : public Renderer {
    public:
        void initialize(RendererAPI *api, RendererConfig config) override;

        void submitScene(Scene *scene) override;

        void submit(Entity *e) override;

        void prepareFrame(Camera* camera) override;

        void render() override;

        void endFrame() override;

        void setTarget(FrameBuffer *target) override;

        void resize(uint32_t width, uint32_t height) override;

    private:
        std::vector<Entity*> renderableEntities;
        std::vector<Entity*> lightEntities;
        RendererAPI* api;
        RendererConfig config;
    };
}


#endif //ICE_FORWARDRENDERER_H
