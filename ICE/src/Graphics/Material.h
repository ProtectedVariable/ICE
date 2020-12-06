//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_MATERIAL_H
#define ICE_MATERIAL_H

#include "Shader.h"

namespace ICE {
    class Material {
    public:
        Material(Shader *shader, const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                 const Eigen::Vector3f &ambient, float alpha);

        Shader *getShader() const;

        void setShader(Shader *shader);

        const Eigen::Vector3f &getAlbedo() const;

        void setAlbedo(const Eigen::Vector3f &albedo);

        const Eigen::Vector3f &getSpecular() const;

        void setSpecular(const Eigen::Vector3f &specular);

        const Eigen::Vector3f &getAmbient() const;

        void setAmbient(const Eigen::Vector3f &ambient);

        float getAlpha() const;

        void setAlpha(float alpha);

    private:
        Shader* shader;
        Eigen::Vector3f albedo, specular, ambient;
        float alpha;
    };
}




#endif //ICE_MATERIAL_H
