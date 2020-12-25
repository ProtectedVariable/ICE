//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_GRAPHICSAPI_H
#define ICE_GRAPHICSAPI_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <Graphics/VertexArray.h>

namespace ICE {
    enum GraphicsAPI {
        None =      0x0,
        OpenGL =    0x1,
    };

    class RendererAPI {
    public:
        virtual void initialize() const = 0;
        virtual void bindDefaultFramebuffer() const = 0;
        virtual void setViewport(int x, int y, int width, int height) const = 0;
        virtual void setClearColor(int r, int g, int b, int a) const = 0;
        virtual void clear() const = 0;
        virtual void renderVertexArray(const VertexArray* va) const = 0;
        virtual void flush() const = 0;
        virtual void finish() const = 0;

        static GraphicsAPI GetAPI() { return api; }
        static RendererAPI* Create();

    private:
        const static GraphicsAPI api = OpenGL; //TODO: Allow for switches
    };
}

#endif //ICE_GRAPHICSAPI_H
