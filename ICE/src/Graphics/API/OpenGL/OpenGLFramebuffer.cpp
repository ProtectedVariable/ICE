//
// Created by Thomas Ibanez on 27.11.20.
//

#include <Util/Logger.h>
#include "OpenGLFramebuffer.h"

namespace ICE {
    void OpenGLFramebuffer::bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, uid);
    }

    void OpenGLFramebuffer::unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::resize(int width, int height) {

    }

    void *OpenGLFramebuffer::getTexture() {
        return static_cast<char*>(0)+texture;
    }

    OpenGLFramebuffer::OpenGLFramebuffer(FrameBufferFormat fmt) {
        glGenFramebuffers(1, &uid);
        glBindFramebuffer(GL_FRAMEBUFFER, uid);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fmt.width, fmt.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);


        GLuint rboDepthStencil;
        glGenRenderbuffers(1, &rboDepthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fmt.width, fmt.height);

        glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil
        );

        GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Logger::Log(Logger::FATAL, "Graphics", "Couldn't create framebuffer object ! (%d)", status);
        }

    }
}