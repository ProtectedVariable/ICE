//
// Created by Thomas Ibanez on 27.11.20.
//

#pragma once

#include <Eigen/Core>
#include <Framebuffer.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

namespace ICE {
class OpenGLFramebuffer : public Framebuffer {
   public:
    OpenGLFramebuffer(FrameBufferFormat fmt);

    void bind() override;

    void unbind() override;

    void resize(int width, int height) override;

    int getTexture() override;

    void bindAttachment(int slot) const override;

    Eigen::Vector4i readPixel(int x, int y) override;

   private:
    GLuint uid;
    GLuint texture;
    GLuint depth;
};
}  // namespace ICE