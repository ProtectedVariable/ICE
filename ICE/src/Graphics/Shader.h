//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_SHADER_H
#define ICE_SHADER_H

#include <string>
#include <Eigen/Dense>

namespace ICE {
    class Shader {
    public:
        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void loadInt(const std::string &name, int v) = 0;
        virtual void loadInts(const std::string &name, int* array, uint32_t size) = 0;

        virtual void loadDouble(const std::string &name, double v) = 0;
        virtual void loadDouble3(const std::string &name, Eigen::Vector3f vec) = 0;
        virtual void loadDouble4(const std::string &name, Eigen::Vector4f vec) = 0;

        virtual void loadMat4(const std::string &name, Eigen::Matrix4f mat) = 0;

        static Shader* Create(const std::string &vertexFile, const std::string &fragmentFile);
        static Shader* Create(const std::string &vertexFile, const std::string &geometryFile, const std::string &fragmentFile);
    };
}

#endif //ICE_SHADER_H
