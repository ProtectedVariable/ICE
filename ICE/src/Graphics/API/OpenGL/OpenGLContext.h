//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLCONTEXT_H
#define ICE_OPENGLCONTEXT_H

#include <Graphics/Context.h>
#include <GLFW/glfw3.h>

namespace ICE {
    class OpenGLContext : public Context {
    public:
        void SwapBuffers() override;

        void WireframeMode() override;

        void FillMode() override;

        void Initialize() override;

        OpenGLContext(GLFWwindow *glfwWindow);

    private:
        GLFWwindow* glfwWindow;
    };
}


#endif //ICE_OPENGLCONTEXT_H
