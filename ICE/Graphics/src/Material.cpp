//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Material.h"

namespace ICE {

    Material::Material() : Material(Eigen::Vector3f(1,1,1),Eigen::Vector3f(1,1,1),Eigen::Vector3f(1,1,1), 1) {}


    Material::Material(const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                       const Eigen::Vector3f &ambient, float alpha) : Material(albedo, specular, ambient, alpha,
                                                                               NO_ASSET_ID, NO_ASSET_ID, NO_ASSET_ID) {}

    Material::Material(const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                       const Eigen::Vector3f &ambient, float alpha, AssetUID diffuseMap, AssetUID specularMap,
                       AssetUID ambientMap) : Material(albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, NO_ASSET_ID){}

    Material::Material(const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                       const Eigen::Vector3f &ambient, float alpha, AssetUID diffuseMap,
                       AssetUID specularMap, AssetUID ambientMap, AssetUID normalMap) : albedo(albedo),
                                                                                                          specular(specular), ambient(ambient),
                                                                                                          alpha(alpha), diffuseMap(diffuseMap), specularMap(specularMap),
                                                                                                          ambientMap(ambientMap), normalMap(normalMap) {}

    const Eigen::Vector3f &Material::getAlbedo() const {
        return albedo;
    }

    void Material::setAlbedo(const Eigen::Vector3f &albedo) {
        Material::albedo = albedo;
    }

    const Eigen::Vector3f &Material::getSpecular() const {
        return specular;
    }

    void Material::setSpecular(const Eigen::Vector3f &specular) {
        Material::specular = specular;
    }

    const Eigen::Vector3f &Material::getAmbient() const {
        return ambient;
    }

    void Material::setAmbient(const Eigen::Vector3f &ambient) {
        Material::ambient = ambient;
    }

    float Material::getAlpha() const {
        return alpha;
    }

    void Material::setAlpha(float alpha) {
        Material::alpha = alpha;
    }

    AssetUID Material::getDiffuseMap() const {
        return diffuseMap;
    }

    void Material::setDiffuseMap(AssetUID diffuseMap) {
        this->diffuseMap = diffuseMap;
    }

    AssetUID Material::getSpecularMap() const {
        return specularMap;
    }

    void Material::setSpecularMap(AssetUID specularMap) {
        this->specularMap = specularMap;
    }

    AssetUID Material::getAmbientMap() const {
        return ambientMap;
    }

    void Material::setAmbientMap(AssetUID ambientMap) {
        this->ambientMap = ambientMap;
    }

    AssetUID Material::getNormalMap() const {
        return normalMap;
    }

    void Material::setNormalMap(AssetUID normalMap) {
        this->normalMap = normalMap;
    }

    std::string Material::getTypeName() {
        return "Material";
    }
}