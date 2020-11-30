//
// Created by Thomas Ibanez on 27.11.20.
//

#ifndef ICE_OPENGLFRAMEBUFFER_H
#define ICE_OPENGLFRAMEBUFFER_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <Graphics/Framebuffer.h>

namespace ICE {
    class OpenGLFramebuffer : public Framebuffer {
    public:
        OpenGLFramebuffer(FrameBufferFormat fmt);

        void bind() override;

        void unbind() override;

        void resize(int width, int height) override;

        void *getTexture() override;
    private:
        GLuint uid;
        GLuint texture;
        GLuint depth;
    };
}


#endif //ICE_OPENGLFRAMEBUFFER_H
