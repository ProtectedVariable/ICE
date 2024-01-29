//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLContext.h"
#include <GLFW/glfw3.h>
#include <GL/gl3w.h>
#include <ICEException.h>

namespace ICE {

OpenGLContext::OpenGLContext(const std::shared_ptr<Window> &window) : m_window(window) {}

void OpenGLContext::swapBuffers() {
    m_window->swapBuffers();
}

void OpenGLContext::wireframeMode() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void OpenGLContext::fillMode() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void OpenGLContext::initialize() {
    m_window->makeContextCurrent();
    bool err = gl3wInit();
    if(err) {
        throw ICEException("Couldn't load OpenGL");
    }
}

}