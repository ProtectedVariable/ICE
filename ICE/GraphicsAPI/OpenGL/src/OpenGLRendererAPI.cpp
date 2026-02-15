//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLRendererAPI.h"

#include <GL/gl3w.h>
#include <Logger.h>

namespace ICE {

void OpenGLRendererAPI::setViewport(int x, int y, int width, int height) const {
    glViewport(x, y, width, height);
}

void OpenGLRendererAPI::setClearColor(float r, float g, float b, float a) const {
    glClearColor(r, g, b, a);
}

void OpenGLRendererAPI::clear() const {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::renderVertexArray(const std::shared_ptr<VertexArray> &va) const {
    glDrawElements(GL_TRIANGLES, va->getIndexCount(), GL_UNSIGNED_INT, 0);
}

void OpenGLRendererAPI::renderVertexArrayInstanced(const std::shared_ptr<VertexArray> &va, uint32_t instance_count) const {
    glDrawElementsInstanced(GL_TRIANGLES, va->getIndexCount(), GL_UNSIGNED_INT, 0, instance_count);
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
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void OpenGLRendererAPI::setDepthMask(bool enable) const {
    glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

void OpenGLRendererAPI::setBackfaceCulling(bool enable) const {
    if (enable) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void OpenGLRendererAPI::checkAndLogErrors() const {
    unsigned int err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        Logger::Log(Logger::ERROR, "Graphics", "OpenGL Error %d", err);
    }
}
}  // namespace ICE