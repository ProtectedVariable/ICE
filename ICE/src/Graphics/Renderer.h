//
// Created by Thomas Ibanez on 17.11.20.
//

#ifndef ICE_RENDERER_H
#define ICE_RENDERER_H

#include <Scene/RenderComponent.h>
#include <Scene/Entity.h>
#include <Graphics/API/GraphicsAPI.h>
#include <Scene/Scene.h>
#include "Context.h"
#include "RendererConfig.h"
#include "Camera.h"
#include "FrameBuffer.h"

namespace ICE {
    class Scene;

    class Renderer {
    public:
        virtual void initialize(RendererAPI* api, RendererConfig config) = 0;
        virtual void submitScene(Scene* scene) = 0;
        virtual void submit(Entity* e) = 0;
        virtual void prepareFrame() = 0;
        virtual void render() = 0;
        virtual void endFrame() = 0;
        virtual void setCamera(const Camera* camera) = 0;
        virtual void setTarget(FrameBuffer* target) = 0;
        virtual void resize(uint32_t width, uint32_t height) = 0;

    };
}


#endif //ICE_RENDERER_H
