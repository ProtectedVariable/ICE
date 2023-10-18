//
// Created by Thomas Ibanez on 27.11.20.
//

#include "OpenGLFramebuffer.h"
#include <Util/Logger.h>
#include <Eigen/src/Core/Matrix.h>
#include <Eigen/Dense>

namespace ICE {
    void OpenGLFramebuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, uid);
    }

    void OpenGLFramebuffer::unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::resize(int width, int height) {
        if(width <= 0) width = 1;
        if(height<= 0) height = 1;
        fmt.width = width;
        fmt.height = height;
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth
        );
    }

    void *OpenGLFramebuffer::getTexture() {
        return static_cast<char*>(0)+texture;
    }

    OpenGLFramebuffer::OpenGLFramebuffer(FrameBufferFormat fmt): fmt(fmt) {
        glGenFramebuffers(1, &uid);
        glBindFramebuffer(GL_FRAMEBUFFER, uid);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fmt.width, fmt.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);


        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fmt.width, fmt.height);

        glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth
        );

        GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::Log(Logger::FATAL, "Graphics", "Couldn't create framebuffer object ! (%d)", status);
        }

    }

    Eigen::Vector4i OpenGLFramebuffer::readPixel(int x, int y) {
        glFlush();
        glFinish();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        auto pixels = Eigen::Vector4i();
        glReadPixels(x, fmt.height-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        return pixels;
    }
}