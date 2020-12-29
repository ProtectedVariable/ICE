//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLRendererAPI.h"
#include <GL/gl3w.h>

namespace ICE {

    void OpenGLRendererAPI::setViewport(int x, int y, int width, int height) const {
        glViewport(x, y, width, height);
    }

    void OpenGLRendererAPI::setClearColor(int r, int g, int b, int a) const {
        glClearColor(r, g, b, a);
    }

    void OpenGLRendererAPI::clear() const {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::renderVertexArray(const VertexArray *va) const {
        va->bind();
        va->getIndexBuffer()->bind();
        //glDrawArrays(GL_TRIANGLES, 0, va->getIndexCount()*3);
        glDrawElements(GL_TRIANGLES, va->getIndexCount(), GL_UNSIGNED_INT, 0);
    }

    void OpenGLRendererAPI::initialize() const {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::flush() const {
        glFlush();
    }

    void OpenGLRendererAPI::finish() const {
        glFinish();
    }

    void OpenGLRendererAPI::bindDefaultFramebuffer() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLRendererAPI::setDepthTest(bool enable) const {
        if(enable)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void OpenGLRendererAPI::setDepthMask(bool enable) const {
        glDepthMask(enable ? GL_TRUE : GL_FALSE);
    }
}