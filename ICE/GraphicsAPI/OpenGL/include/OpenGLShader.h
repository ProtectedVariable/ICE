//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLSHADER_H
#define ICE_OPENGLSHADER_H

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <Shader.h>
#include <unordered_map>
#include <string>

namespace ICE {
    class OpenGLShader : public Shader {
    public:
        void bind() const override;

        void unbind() const override;

        void loadInt(const std::string &name, int v) override;

        void loadInts(const std::string &name, int *array, uint32_t size) override;

        void loadFloat(const std::string &name, float v) override;

        void loadFloat3(const std::string &name, Eigen::Vector3f vec) override;

        void loadFloat4(const std::string &name, Eigen::Vector4f vec) override;

        void loadMat4(const std::string &name, Eigen::Matrix4f mat) override;

        OpenGLShader(const std::string &vertexFile, const std::string &geoFile, const std::string &fragmentFile);

        OpenGLShader(const std::string &vertexFile, const std::string &fragmentFile);

        void load() override {}

        void unload() override {}

    private:

        GLint getLocation(const std::string &name);

        uint32_t programID;
        std::unordered_map<std::string, uint32_t> locations;
    };
}


#endif //ICE_OPENGLSHADER_H
