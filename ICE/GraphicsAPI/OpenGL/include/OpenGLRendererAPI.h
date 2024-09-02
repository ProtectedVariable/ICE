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

        void flush() const override;

        void finish() const override;

        void bindDefaultFramebuffer() const override;

        void setDepthTest(bool enable) const override;

        void setDepthMask(bool enable) const override;

        void setBackfaceCulling(bool enable) const override;

        void checkAndLogErrors() const override;
    };
}


#endif //ICE_OPENGLRENDERERAPI_H
