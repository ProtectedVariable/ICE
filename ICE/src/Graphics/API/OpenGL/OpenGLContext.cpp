//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLContext.h"
#include <Graphics/Context.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>

ICE::OpenGLContext::OpenGLContext(GLFWwindow *glfwWindow) : glfwWindow(glfwWindow) {}

void ICE::OpenGLContext::SwapBuffers() {
    glfwSwapBuffers(glfwWindow);
}

void ICE::OpenGLContext::WireframeMode() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void ICE::OpenGLContext::FillMode() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ICE::OpenGLContext::Initialize() {
    glfwMakeContextCurrent(glfwWindow);
}