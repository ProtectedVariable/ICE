//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_GRAPHICSAPI_H
#define ICE_GRAPHICSAPI_H

#include <Graphics/VertexArray.h>

namespace ICE {
    enum GraphicsAPI {
        None =      0x0,
        OpenGL =    0x1,
    };

    class RendererAPI {
    public:
        virtual void SetViewport(int x, int y, int width, int height) = 0;
        virtual void SetClearColor(int r, int g, int b, int a) = 0;
        virtual void Clear() = 0;

        virtual void RenderVertexArray(VertexArray &va) = 0;
        static GraphicsAPI GetAPI() { return api; }

    private:
        static GraphicsAPI api;
    };
}

#endif //ICE_GRAPHICSAPI_H
