#include "GLFWWindow.h"

#include <GLFW/glfw3.h>
#include <ICEException.h>

#include "GLFWKeyboardHandler.h"
#include "GLFWMouseHandler.h"

namespace ICE {

GLFWWindow::GLFWWindow(int width, int height, const std::string& title) {
    if (!glfwInit())
        throw ICEException("Couldn't init GLFW");
// Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    // Create window with graphics context
    m_handle = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (m_handle == NULL)
        throw ICEException("Couldn't create window");

    m_mouse_handler = std::make_shared<GLFWMouseHandler>(*this);
    m_keyboard_handler = std::make_shared<GLFWKeyboardHandler>(*this);

    glfwSetWindowUserPointer(m_handle, this);
}

std::pair<std::shared_ptr<MouseHandler>, std::shared_ptr<KeyboardHandler>> GLFWWindow::getInputHandlers() const {
    return {m_mouse_handler, m_keyboard_handler};
}

void* GLFWWindow::getHandle() const {
    return static_cast<void*>(m_handle);
}

bool GLFWWindow::shouldClose() {
    return glfwWindowShouldClose(m_handle);
}

void GLFWWindow::pollEvents() {
    glfwPollEvents();
}
void GLFWWindow::swapBuffers() {
    glfwSwapBuffers(m_handle);
}

void GLFWWindow::getFramebufferSize(int* width, int* height) {
    glfwGetFramebufferSize(m_handle, width, height);
}

void GLFWWindow::setSwapInterval(int interval) {
    glfwSwapInterval(interval);
}

void GLFWWindow::makeContextCurrent() {
    glfwMakeContextCurrent(m_handle);
}
}  // namespace ICE