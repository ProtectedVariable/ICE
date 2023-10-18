//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_MATERIAL_H
#define ICE_MATERIAL_H

#include "Shader.h"
#include "Texture.h"

namespace ICE {
    class Material : public Asset {
    public:
        Material();

        Material(const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                 const Eigen::Vector3f &ambient, float alpha);

        Material(const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                 const Eigen::Vector3f &ambient, float alpha, AssetUID  diffuseMap, AssetUID  specularMap, AssetUID  ambientMap);

        Material(const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                 const Eigen::Vector3f &ambient, float alpha, AssetUID  diffuseMap, AssetUID  specularMap, AssetUID  ambientMap,
                 AssetUID  normalMap);

        const Eigen::Vector3f &getAlbedo() const;
        void setAlbedo(const Eigen::Vector3f &albedo);

        const Eigen::Vector3f &getSpecular() const;
        void setSpecular(const Eigen::Vector3f &specular);

        const Eigen::Vector3f &getAmbient() const;
        void setAmbient(const Eigen::Vector3f &ambient);

        float getAlpha() const;
        void setAlpha(float alpha);

        AssetUID getDiffuseMap() const;
        void setDiffuseMap(AssetUID diffuseMap);

        AssetUID getSpecularMap() const;
        void setSpecularMap(AssetUID specularMap);

        AssetUID getAmbientMap() const;
        void setAmbientMap(AssetUID ambientMap);

        AssetUID getNormalMap() const;
        void setNormalMap(AssetUID normalMap);

        std::string getTypeName() override;

    private:
        Eigen::Vector3f albedo, specular, ambient;
        float alpha;
        AssetUID diffuseMap;
        AssetUID specularMap;
        AssetUID ambientMap;
        AssetUID normalMap;
    };
}




#endif //ICE_MATERIAL_H
