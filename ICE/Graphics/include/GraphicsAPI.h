//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_GRAPHICSAPI_H
#define ICE_GRAPHICSAPI_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <VertexArray.h>

namespace ICE {
enum GraphicsAPI {
    None = 0x0,
    OpenGL = 0x1,
};

class RendererAPI {
   public:
    virtual void initialize() const = 0;
    virtual void bindDefaultFramebuffer() const = 0;
    virtual void setViewport(int x, int y, int width, int height) const = 0;
    virtual void setClearColor(float r, float g, float b, float a) const = 0;
    virtual void clear() const = 0;
    virtual void renderVertexArray(const std::shared_ptr<VertexArray> &va) const = 0;
    virtual void renderVertexArrayInstanced(const std::shared_ptr<VertexArray> &va, uint32_t instance_count) const = 0;
    virtual void flush() const = 0;
    virtual void finish() const = 0;
    virtual void setDepthTest(bool enable) const = 0;
    virtual void setDepthMask(bool enable) const = 0;
    virtual void setBackfaceCulling(bool enable) const = 0;
    virtual void checkAndLogErrors() const = 0;

    static GraphicsAPI GetAPI() { return api; }
   private:
    static GraphicsAPI api;
};

}  // namespace ICE

#endif  //ICE_GRAPHICSAPI_H
