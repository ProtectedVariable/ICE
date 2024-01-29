//
// Created by Thomas Ibanez on 19.11.20.
//

#pragma once

#include <Eigen/Dense>

namespace ICE {
struct FrameBufferFormat {
    int width, height, samples;
};

class Framebuffer {
   public:
    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual void resize(int width, int height) = 0;
    virtual void* getTexture() = 0;
    virtual Eigen::Vector4i readPixel(int x, int y) = 0;

    FrameBufferFormat getFormat() const { return format; }
    Framebuffer(FrameBufferFormat fmt) : format(fmt) {}

   protected:
    FrameBufferFormat format;
};
}  // namespace ICE
