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
        void initialize() override;

        void setViewport(int x, int y, int width, int height) override;

        void setClearColor(int r, int g, int b, int a) override;

        void clear() override;

        void renderVertexArray(const VertexArray* va) override;

        void flush() override;

        void finish() override;
    };
}


#endif //ICE_OPENGLRENDERERAPI_H
