#pragma once

#include <GLFWWindow.h>

#include <memory>

#include "MouseHandler.h"

namespace ICE {
class GLFWMouseHandler : public MouseHandler {
   public:
    GLFWMouseHandler(const GLFWWindow &window) {
        auto win_ptr = (GLFWwindow *) window.getHandle();
        glfwSetCursorPosCallback(win_ptr, [](GLFWwindow *window, double xpos, double ypos) {
            GLFWWindow *w = (GLFWWindow *) glfwGetWindowUserPointer(window);
            auto self = (GLFWMouseHandler *) w->getInputHandlers().first.get();
            for (const auto &f : self->m_mousemove_callbacks) {
                f(xpos, ypos);
            }
        });
    }
};
}  // namespace ICE
