#pragma once

#include <GLFWWindow.h>

#include <memory>

#include "MouseHandler.h"

namespace ICE {
class GLFWMouseHandler : public MouseHandler {
   public:
    GLFWMouseHandler(const GLFWWindow &window) : m_ptr((GLFWwindow *) window.getHandle()) {
        glfwSetCursorPosCallback(m_ptr, [](GLFWwindow *window, double xpos, double ypos) {
            GLFWWindow *w = (GLFWWindow *) glfwGetWindowUserPointer(window);
            auto self = (GLFWMouseHandler *) w->getInputHandlers().first.get();
            for (const auto &f : self->m_mousemove_callbacks) {
                f(xpos, ypos);
            }
        });
    }

    void setGrabMouse(bool grab) override {
        m_grab = grab;
        if (grab) {
            glfwSetInputMode(m_ptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(m_ptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

   private:
    bool m_grab = false;
    GLFWwindow *m_ptr;
};
}  // namespace ICE
