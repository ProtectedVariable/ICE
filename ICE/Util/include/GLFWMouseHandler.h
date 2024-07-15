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

        glfwSetMouseButtonCallback(m_ptr, [](GLFWwindow *window, int button, int action, int mods) {
            GLFWWindow *w = (GLFWWindow *) glfwGetWindowUserPointer(window);
            auto self = (GLFWMouseHandler *) w->getInputHandlers().first.get();
            ButtonAction act = action == GLFW_PRESS ? ButtonAction::PRESS : ButtonAction::RELEASE;
            MouseButton btn = MouseButton::OTHER_MOUSE_BUTTON;
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    btn = MouseButton::LEFT_MOUSE_BUTTON;
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    btn = MouseButton::RIGHT_MOUSE_BUTTON;
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    btn = MouseButton::MIDDLE_MOUSE_BUTTON;
                    break;
                default:
                    btn = MouseButton::OTHER_MOUSE_BUTTON;
                    break;
            }
            for (const auto &f : self->m_mousebutton_callbacks) {
                f(btn, act);
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
