//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLShader.h"

#include <GL/gl3w.h>
#include <Logger.h>

#include <fstream>

namespace ICE {
void OpenGLShader::bind() const {
    glUseProgram(programID);
}

void OpenGLShader::unbind() const {
    glUseProgram(0);
}

void OpenGLShader::loadInt(const std::string &name, int v) {
    glUniform1i(getLocation(name), v);
}

void OpenGLShader::loadInts(const std::string &name, int *array, uint32_t size) {
    glUniform1iv(getLocation(name), size, array);
}

void OpenGLShader::loadFloat(const std::string &name, float v) {
    glUniform1f(getLocation(name), v);
}

void OpenGLShader::loadFloat2(const std::string &name, Eigen::Vector2f vec) {
    glUniform2f(getLocation(name), vec.x(), vec.y());
}

void OpenGLShader::loadFloat3(const std::string &name, Eigen::Vector3f vec) {
    glUniform3f(getLocation(name), vec.x(), vec.y(), vec.z());
}

void OpenGLShader::loadFloat4(const std::string &name, Eigen::Vector4f vec) {
    glUniform4f(getLocation(name), vec.x(), vec.y(), vec.z(), vec.w());
}

void OpenGLShader::loadMat4(const std::string &name, Eigen::Matrix4f mat) {
    glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, mat.data());
}

GLint OpenGLShader::getLocation(const std::string &name) {
    if (this->locations.find(name) == this->locations.end()) {
        GLint location = glGetUniformLocation(programID, name.c_str());
        locations[name] = static_cast<unsigned int>(location);
    }
    return locations[name];
}

bool compileShader(GLenum type, const std::string &source, GLint *shader) {
    *shader = glCreateShader(type);
    const char *src = (source.c_str());
    glShaderSource(*shader, 1, &src, 0);

    glCompileShader(*shader);

    GLint compileStatus = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus != GL_TRUE) {
        GLint maxLength = 0;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(*shader, maxLength, &maxLength, &errorLog[0]);

        Logger::Log(Logger::FATAL, "Graphics", "Shader compilation error: %s", errorLog.data());
    }

    return compileStatus == GL_TRUE;
}

//TODO: Another ctor for tesselation control and evaluation shaders
OpenGLShader::OpenGLShader(const std::string &vertex_src, const std::string &geo_src, const std::string &fragment_src) {
    //TODO: Better tracing in case of errors, cleanup
    this->programID = glCreateProgram();

    GLint vertexShader;
    Logger::Log(Logger::VERBOSE, "Graphics", "Compiling vertex shader...");
    if (!compileShader(GL_VERTEX_SHADER, vertex_src, &vertexShader)) {
        Logger::Log(Logger::FATAL, "Graphics", "Error while compiling vertex shader");
    }
    glAttachShader(programID, vertexShader);

    GLint fragmentShader;
    Logger::Log(Logger::VERBOSE, "Graphics", "Compiling fragment shader...");
    if (!compileShader(GL_FRAGMENT_SHADER, fragment_src, &fragmentShader)) {
        Logger::Log(Logger::FATAL, "Graphics", "Error while compiling fragment shader");
    }
    glAttachShader(programID, fragmentShader);

    if (!geo_src.empty()) {
        GLint geoShader;
        Logger::Log(Logger::VERBOSE, "Graphics", "Compiling geometry shader...");
        if (!compileShader(GL_GEOMETRY_SHADER, geo_src, &geoShader)) {
            Logger::Log(Logger::FATAL, "Graphics", "Error while compiling geometry shader");
        }
        glAttachShader(programID, geoShader);
    }

    glLinkProgram(programID);
    GLint linkStatus = 0;
    glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        Logger::Log(Logger::FATAL, "Graphics", "Error while linking shader");
        GLint maxLength = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(programID, maxLength, &maxLength, &errorLog[0]);
        Logger::Log(Logger::FATAL, "Graphics", "Shader linking error: %s", errorLog.data());
    }
}

OpenGLShader::OpenGLShader(const std::string &vertex_src, const std::string &fragment_src) : OpenGLShader(vertex_src, "", fragment_src) {
}
}  // namespace ICE