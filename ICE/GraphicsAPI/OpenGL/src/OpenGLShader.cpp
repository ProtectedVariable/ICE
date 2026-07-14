//
// Created by Thomas Ibanez on 20.11.20.
//

#include "OpenGLShader.h"

#include <GL/gl3w.h>
#include <Logger.h>

#include <fstream>

namespace ICE {

OpenGLShader::OpenGLShader(const Shader &shader_asset) {
    m_programID = glCreateProgram();
    Logger::Log(Logger::VERBOSE, "Graphics", "Compiling shader...");

    std::vector<GLuint> stage_shaders;
    for (const auto& [stage, source] : shader_asset.getStageSources()) {
        stage_shaders.push_back(compileAndAttachStage(stage, source.second));
    }

    glLinkProgram(m_programID);
    GLint linkStatus = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        Logger::Log(Logger::FATAL, "Graphics", "Error while linking shader");
        GLint maxLength = 0;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(m_programID, maxLength, &maxLength, &errorLog[0]);
        Logger::Log(Logger::FATAL, "Graphics", "Shader linking error: %s", errorLog.data());
    }

    // Stage objects are no longer needed once linked into the program. Skip 0, which
    // marks a stage that failed to compile (and so was never attached).
    for (GLuint shader : stage_shaders) {
        if (shader != 0) {
            glDetachShader(m_programID, shader);
            glDeleteShader(shader);
        }
    }
}

OpenGLShader::~OpenGLShader() {
    glDeleteProgram(m_programID);
}

void OpenGLShader::bind() const {
    glUseProgram(m_programID);
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

void OpenGLShader::loadFloat2(const std::string &name, const Eigen::Vector2f &vec) {
    glUniform2f(getLocation(name), vec.x(), vec.y());
}

void OpenGLShader::loadFloat3(const std::string &name, const Eigen::Vector3f &vec) {
    glUniform3f(getLocation(name), vec.x(), vec.y(), vec.z());
}

void OpenGLShader::loadFloat4(const std::string &name, const Eigen::Vector4f &vec) {
    glUniform4f(getLocation(name), vec.x(), vec.y(), vec.z(), vec.w());
}

void OpenGLShader::loadMat4(const std::string &name, const Eigen::Matrix4f &mat) {
    glUniformMatrix4fv(getLocation(name), 1, GL_FALSE, mat.data());
}

void OpenGLShader::loadMat4v(const std::string &name, const Eigen::Matrix4f *data, uint32_t count) {
    // std::vector<Matrix4f> / a Matrix4f array is contiguous, column-major -- exactly what
    // glUniformMatrix4fv expects for a mat4[] uniform, so one call uploads the whole array.
    glUniformMatrix4fv(getLocation(name), count, GL_FALSE, data->data());
}

GLint OpenGLShader::getLocation(const std::string &name) {
    // Single hash lookup on the hot path (was contains + operator[] insert + operator[]).
    auto it = m_locations.find(name);
    if (it != m_locations.end()) {
        return static_cast<GLint>(it->second);
    }
    GLint location = glGetUniformLocation(m_programID, name.c_str());
    m_locations.emplace(name, static_cast<unsigned int>(location));
    return location;
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

GLuint OpenGLShader::compileAndAttachStage(ShaderStage stage, const std::string &source) {
    GLint shader;
    Logger::Log(Logger::VERBOSE, "Graphics", "\t + Compiling shader stage...");
    if (!compileShader(stageToGLStage(stage), source, &shader)) {
        // Don't attach a stage that failed to compile: attaching it only guarantees the
        // link fails too, producing a broken-but-alive program. Return 0 to signal failure.
        Logger::Log(Logger::FATAL, "Graphics", "Error while compiling shader stage");
        glDeleteShader(shader);
        return 0;
    }
    glAttachShader(m_programID, shader);
    return static_cast<GLuint>(shader);
}


GLenum OpenGLShader::stageToGLStage(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderStage::Fragment:
            return GL_FRAGMENT_SHADER;
        case ShaderStage::Geometry:
            return GL_GEOMETRY_SHADER;
        case ShaderStage::TessControl:
            return GL_TESS_CONTROL_SHADER;
        case ShaderStage::TessEval:
            return GL_TESS_EVALUATION_SHADER;
        case ShaderStage::Compute:
            return GL_COMPUTE_SHADER;
    }
    Logger::Log(Logger::FATAL, "Graphics", "Unknown shader stage %d", static_cast<int>(stage));
    return GL_VERTEX_SHADER;
}

}  // namespace ICE