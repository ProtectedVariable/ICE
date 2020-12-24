//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_MATERIAL_H
#define ICE_MATERIAL_H

#include "Shader.h"
#include "Texture.h"

namespace ICE {
    class Material {
    public:
        Material(Shader *shader, const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                 const Eigen::Vector3f &ambient, float alpha);

        Material(Shader *shader, const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                 const Eigen::Vector3f &ambient, float alpha, const Texture* diffuseMap, const Texture* specularMap, const Texture* ambientMap);

        Material(Shader *shader, const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                 const Eigen::Vector3f &ambient, float alpha, const Texture* diffuseMap, const Texture* specularMap, const Texture* ambientMap,
                 const Texture* normalMap);

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

        const Texture *getDiffuseMap() const;
        void setDiffuseMap(const Texture *diffuseMap);

        const Texture *getSpecularMap() const;
        void setSpecularMap(const Texture *specularMap);

        const Texture *getAmbientMap() const;
        void setAmbientMap(const Texture *ambientMap);

        const Texture *getNormalMap() const;
        void setNormalMap(const Texture *normalMap);

    private:
        Shader* shader;
        Eigen::Vector3f albedo, specular, ambient;
        float alpha;
        const Texture* diffuseMap;
        const Texture* specularMap;
        const Texture* ambientMap;
        const Texture* normalMap;
    };
}




#endif //ICE_MATERIAL_H
