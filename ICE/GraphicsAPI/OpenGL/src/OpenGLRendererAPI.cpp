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

void OpenGLRendererAPI::setDepthFunc(DepthFunc func) const {
    glDepthFunc(func == DepthFunc::LEqual ? GL_LEQUAL : GL_LESS);
}

void OpenGLRendererAPI::setBlend(bool enable) const {
    if (enable) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
}

void OpenGLRendererAPI::beginGPUTimer() const {
    if (!m_gpu_query_init) {
        glGenQueries(2, m_gpu_query);
        m_gpu_query_init = true;
    }
    glBeginQuery(GL_TIME_ELAPSED, m_gpu_query[m_gpu_query_idx]);
}

double OpenGLRendererAPI::endGPUTimer() const {
    glEndQuery(GL_TIME_ELAPSED);
    m_gpu_query_used[m_gpu_query_idx] = true;

    // Read the other buffer's result (last frame's query): one frame old, so it's ready and
    // reading it doesn't stall.
    const int other = 1 - m_gpu_query_idx;
    if (m_gpu_query_used[other]) {
        GLuint64 elapsed_ns = 0;
        glGetQueryObjectui64v(m_gpu_query[other], GL_QUERY_RESULT, &elapsed_ns);
        m_last_gpu_ms = static_cast<double>(elapsed_ns) / 1.0e6;
    }
    m_gpu_query_idx = other;
    return m_last_gpu_ms;
}

void OpenGLRendererAPI::setBackfaceCulling(bool enable) const {
    if (enable) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

void OpenGLRendererAPI::checkAndLogErrors() const {
    // glDebugMessageCallback would be nicer but it is not core until GL 4.3; the engine
    // targets 4.1 (macOS caps there), so decode the enum to a readable name instead of
    // logging a bare number. Callers should only drain this in debug builds.
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        const char *name;
        switch (err) {
            case GL_INVALID_ENUM: name = "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: name = "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: name = "GL_INVALID_OPERATION"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            case GL_OUT_OF_MEMORY: name = "GL_OUT_OF_MEMORY"; break;
            default: name = "GL_UNKNOWN"; break;
        }
        Logger::Log(Logger::ERROR, "Graphics", "OpenGL error: %s (0x%x)", name, err);
    }
}
}  // namespace ICE