//
// Created by Thomas Ibanez on 17.11.20.
//

#pragma once

#include <AssetBank.h>
#include <Entity.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Context.h"
#include "Framebuffer.h"
#include "RendererConfig.h"

namespace ICE {
class Scene;

class Renderer {
   public:
    virtual void submit(Entity e) = 0;
    virtual void prepareFrame(Camera &camera) = 0;
    virtual void render() = 0;
    virtual void endFrame() = 0;
    virtual void setTarget(const std::shared_ptr<Framebuffer> &target) = 0;
    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual void setClearColor(Eigen::Vector4f clearColor) = 0;
};
}  // namespace ICE
