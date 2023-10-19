//
// Created by Thomas Ibanez on 17.11.20.
//

#ifndef ICE_RENDERER_H
#define ICE_RENDERER_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <GraphicsAPI.h>
#include "Context.h"
#include "RendererConfig.h"
#include "Camera.h"
#include "Framebuffer.h"
#include <Entity.h>
#include <AssetBank.h>

namespace ICE {
    class Scene;

    class Renderer {
    public:
        virtual void initialize(RendererConfig config, AssetBank* assetBank) = 0;
        virtual void submitScene(Scene* scene) = 0;
        virtual void submit(Entity e) = 0;
        virtual void prepareFrame(Camera& camera) = 0;
        virtual void render() = 0;
        virtual void endFrame() = 0;
        virtual void setTarget(Framebuffer* target) = 0;
        virtual void resize(uint32_t width, uint32_t height) = 0;
        virtual void setClearColor(Eigen::Vector4f clearColor) = 0;
    };
}


#endif //ICE_RENDERER_H
