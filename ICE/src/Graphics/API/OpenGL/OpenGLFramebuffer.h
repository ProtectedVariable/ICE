//
// Created by Thomas Ibanez on 27.11.20.
//

#ifndef ICE_OPENGLFRAMEBUFFER_H
#define ICE_OPENGLFRAMEBUFFER_H

#include <Graphics/Framebuffer.h>
#include <OpenGL/gl3.h>

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
    };
}


#endif //ICE_OPENGLFRAMEBUFFER_H
