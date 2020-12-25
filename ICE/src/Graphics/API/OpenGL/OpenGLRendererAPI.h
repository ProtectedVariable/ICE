//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLRENDERERAPI_H
#define ICE_OPENGLRENDERERAPI_H

#include <GL/gl3w.h>
#include <Graphics/API/GraphicsAPI.h>

namespace ICE {
    class OpenGLRendererAPI : public RendererAPI {
    public:
        void initialize() const override;

        void setViewport(int x, int y, int width, int height) const override;

        void setClearColor(int r, int g, int b, int a) const override;

        void clear() const override;

        void renderVertexArray(const VertexArray* va) const override;

        void flush() const override;

        void finish() const override;

        void bindDefaultFramebuffer() const override;
    };
}


#endif //ICE_OPENGLRENDERERAPI_H
