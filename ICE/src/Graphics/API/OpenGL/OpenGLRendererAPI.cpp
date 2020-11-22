//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLRendererAPI.h"
#include <OpenGL/gl3.h>

void ICE::OpenGLRendererAPI::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void ICE::OpenGLRendererAPI::setClearColor(int r, int g, int b, int a) {
    glClearColor(r, g, b, a);
}

void ICE::OpenGLRendererAPI::clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ICE::OpenGLRendererAPI::renderVertexArray(const ICE::VertexArray* va) {
    va->bind();
    glDrawElements(GL_TRIANGLES, va->getIndexCount(), GL_UNSIGNED_INT, nullptr);
}

void ICE::OpenGLRendererAPI::initialize() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
}
