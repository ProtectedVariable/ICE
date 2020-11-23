//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLShader.h"
#include <OpenGL/gl3.h>
#include <fstream>
#include <iostream>

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

    void OpenGLShader::loadDouble(const std::string &name, double v) {
        glUniform1d(getLocation(name), v);
    }

    void OpenGLShader::loadDouble3(const std::string &name, Eigen::Vector3f vec) {
        glUniform3d(getLocation(name), vec.x(), vec.y(), vec.z());
    }

    void OpenGLShader::loadDouble4(const std::string &name, Eigen::Vector4f vec) {
        glUniform4d(getLocation(name), vec.x(), vec.y(), vec.z(), vec.w());
    }

    void OpenGLShader::loadMat4(const std::string &name, Eigen::Matrix4f mat) {
        glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, mat.data());
    }

    GLint OpenGLShader::getLocation(const std::string &name) {
        if(this->locations.find(name) == this->locations.end()) {
            GLint location = glGetUniformLocation(programID, name.c_str());
            locations[name] = location;
        }
        return locations[name];
    }


    bool compileShader(GLenum type, const std::string &source, GLint* shader) {
        std::ifstream vs(source);
        std::stringstream shaderBuffer;
        shaderBuffer << vs.rdbuf();
        vs.close();
        const char* shaderSource = shaderBuffer.str().c_str();

        *shader = glCreateShader(type);
        glShaderSource(*shader, 1, &shaderSource, 0);

        glCompileShader(*shader);

        GLint compileStatus = 0;
        glGetShaderiv(*shader, GL_COMPILE_STATUS, &compileStatus);
        return compileStatus == GL_TRUE;
    }

    //TODO: Another ctor for tesselation control and evaluation shaders
    OpenGLShader::OpenGLShader(const std::string &vertexFile, const std::string &geoFile, const std::string &fragmentFile) {
        //TODO: Better tracing in case of errors, cleanup
        this->programID = glCreateProgram();

        GLint vertexShader;
        compileShader(GL_VERTEX_SHADER, vertexFile, &vertexShader) ?: printf("Error while compiling vertex shader");
        glAttachShader(programID, vertexShader);

        GLint fragmentShader;
        compileShader(GL_FRAGMENT_SHADER, fragmentFile, &fragmentShader) ?: printf("Error while compiling fragment shader");
        glAttachShader(programID, fragmentShader);

        if(!geoFile.empty()){
            GLint geoShader;
            compileShader(GL_GEOMETRY_SHADER, geoFile, &geoShader) ?: printf("Error while compiling geometric shader");
            glAttachShader(programID, geoShader);
        }

        glLinkProgram(programID);
        GLint linkStatus = 0;
        glGetProgramiv(programID, GL_LINK_STATUS, &linkStatus);
        if(linkStatus == GL_FALSE) {
            printf("Error while linking shader");
        }
    }

    OpenGLShader::OpenGLShader(const std::string &vertexFile, const std::string &fragmentFile): OpenGLShader(vertexFile, "", fragmentFile) {}
}