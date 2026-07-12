//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLRENDERERAPI_H
#define ICE_OPENGLRENDERERAPI_H

#include <GL/gl3w.h>
#include "GraphicsAPI.h"

namespace ICE {
    class OpenGLRendererAPI : public RendererAPI {
    public:
        void initialize() const override;

        void setViewport(int x, int y, int width, int height) const override;

        void setClearColor(float r, float g, float b, float a) const override;

        void clear() const override;

        void renderVertexArray(const std::shared_ptr<VertexArray> &va) const override;

        void renderVertexArrayInstanced(const std::shared_ptr<VertexArray> &va, uint32_t instance_count) const override;

        void flush() const override;

        void finish() const override;

        void bindDefaultFramebuffer() const override;

        void setDepthTest(bool enable) const override;

        void setDepthMask(bool enable) const override;

        void setDepthFunc(DepthFunc func) const override;

        void setBlend(bool enable) const override;

        void beginGPUTimer() const override;
        double endGPUTimer() const override;

        void setBackfaceCulling(bool enable) const override;

        void checkAndLogErrors() const override;

    private:
        // Double-buffered GL_TIME_ELAPSED query state (GPU state, hence mutable behind the
        // const API).
        mutable GLuint m_gpu_query[2] = {0, 0};
        mutable bool m_gpu_query_used[2] = {false, false};
        mutable int m_gpu_query_idx = 0;
        mutable bool m_gpu_query_init = false;
        mutable double m_last_gpu_ms = 0.0;
    };
}


#endif //ICE_OPENGLRENDERERAPI_H
