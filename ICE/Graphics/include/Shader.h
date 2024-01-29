//
// Created by Thomas Ibanez on 20.11.20.
//

#ifndef ICE_SHADER_H
#define ICE_SHADER_H

#include <string>
#include <Eigen/Dense>
#include <Asset.h>

namespace ICE {
    class Shader : public Asset {
    public:
        virtual void bind() const = 0;
        virtual void unbind() const = 0;

        virtual void loadInt(const std::string &name, int v) = 0;
        virtual void loadInts(const std::string &name, int* array, uint32_t size) = 0;

        virtual void loadFloat(const std::string &name, float v) = 0;
        virtual void loadFloat3(const std::string &name, Eigen::Vector3f vec) = 0;
        virtual void loadFloat4(const std::string &name, Eigen::Vector4f vec) = 0;

        virtual void loadMat4(const std::string &name, Eigen::Matrix4f mat) = 0;

        std::string getTypeName() const override {
            return "Shader";
        };

        AssetType getType() const override {
            return AssetType::EShader;
        };

    };
}

#endif //ICE_SHADER_H
