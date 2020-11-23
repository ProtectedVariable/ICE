//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_OPENGLSHADER_H
#define ICE_OPENGLSHADER_H

#include <Graphics/Shader.h>
#include <unordered_map>
#include <string>
#include <OpenGL/gl3.h>

namespace ICE {
    class OpenGLShader : public Shader {
    public:
        void bind() const override;

        void unbind() const override;

        void loadInt(const std::string &name, int v) override;

        void loadInts(const std::string &name, int *array, uint32_t size) override;

        void loadDouble(const std::string &name, double v) override;

        void loadDouble3(const std::string &name, Eigen::Vector3f vec) override;

        void loadDouble4(const std::string &name, Eigen::Vector4f vec) override;

        void loadMat4(const std::string &name, Eigen::Matrix4f mat) override;

        OpenGLShader(const std::string &vertexFile, const std::string &geoFile, const std::string &fragmentFile);

        OpenGLShader(const std::string &vertexFile, const std::string &fragmentFile);

    private:

        GLint getLocation(const std::string &name);

        uint32_t programID;
        std::unordered_map<std::string, uint32_t> locations;
    };
}


#endif //ICE_OPENGLSHADER_H
