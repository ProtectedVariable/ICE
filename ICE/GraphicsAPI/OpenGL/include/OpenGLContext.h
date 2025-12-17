//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLCONTEXT_H
#define ICE_OPENGLCONTEXT_H

#include <Context.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <Window.h>

#include <memory>

namespace ICE {
class OpenGLContext : public Context {
   public:
    void swapBuffers() override;

    void wireframeMode() override;

    void fillMode() override;

    void initialize() override;

    OpenGLContext(const std::shared_ptr<Window> &window);

   private:
    std::shared_ptr<Window> m_window;
};
}  // namespace ICE

#endif  //ICE_OPENGLCONTEXT_H
