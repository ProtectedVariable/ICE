//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLContext.h"
#include <GLFW/glfw3.h>
#include <GL/gl3w.h>

ICE::OpenGLContext::OpenGLContext(GLFWwindow *glfwWindow) : glfwWindow(glfwWindow) {}

void ICE::OpenGLContext::swapBuffers() {
    glfwSwapBuffers(glfwWindow);
}

void ICE::OpenGLContext::wireframeMode() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void ICE::OpenGLContext::fillMode() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ICE::OpenGLContext::initialize() {
    glfwMakeContextCurrent(glfwWindow);
}