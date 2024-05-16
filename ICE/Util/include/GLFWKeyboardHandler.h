#pragma once

#include <GLFWWindow.h>

#include <memory>

#include "KeyboardHandler.h"

namespace ICE {
class GLFWKeyboardHandler : public KeyboardHandler {
   public:
    GLFWKeyboardHandler(const GLFWWindow &window) {
        auto win_ptr = (GLFWwindow *) window.getHandle();
        glfwSetKeyCallback(win_ptr, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
            GLFWWindow *w = (GLFWWindow *) glfwGetWindowUserPointer(window);
            auto self = (GLFWKeyboardHandler*)w->getInputHandlers().second.get();
            if (action == GLFW_PRESS) {
                for (const auto &f : self->m_keypress_callbacks) {
                    f(static_cast<Key>(key));
                }
            } else if (action == GLFW_RELEASE) {
                for (const auto &f : self->m_keyrelease_callbacks) {
                    f(static_cast<Key>(key));
                }
            }
        });
    }
};
}  // namespace ICE
