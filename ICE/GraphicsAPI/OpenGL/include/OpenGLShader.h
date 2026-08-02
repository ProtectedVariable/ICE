//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLSHADER_H
#define ICE_OPENGLSHADER_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <Shader.h>
#include <ShaderProgram.h>

#include <string>
#include <unordered_map>

namespace ICE {
class OpenGLShader : public ShaderProgram {
   public:
    explicit OpenGLShader(const Shader &shader_asset);
    ~OpenGLShader() override;

    // Owns a GL program object; copying would double-delete it.
    OpenGLShader(const OpenGLShader &) = delete;
    OpenGLShader &operator=(const OpenGLShader &) = delete;

    void bind() const override;

    void unbind() const override;

    void loadInt(const std::string &name, int v) override;

    void loadInts(const std::string &name, int *array, uint32_t size) override;

    void loadFloat(const std::string &name, float v) override;

    void loadFloat2(const std::string &name, const Eigen::Vector2f &vec) override;

    void loadFloat3(const std::string &name, const Eigen::Vector3f &vec) override;

    void loadFloat4(const std::string &name, const Eigen::Vector4f &vec) override;

    void loadMat4(const std::string &name, const Eigen::Matrix4f &mat) override;

    void loadMat4v(const std::string &name, const Eigen::Matrix4f *data, uint32_t count) override;

   private:
    GLint getLocation(const std::string &name);

    // Compiles and attaches a stage, returning its GL shader name so the caller can
    // detach and delete it after linking.
    GLuint compileAndAttachStage(ShaderStage stage, const std::string &source);

    GLenum stageToGLStage(ShaderStage stage);

    uint32_t m_programID;
    std::unordered_map<std::string, uint32_t> m_locations;
};
}  // namespace ICE

#endif  //ICE_OPENGLSHADER_H
