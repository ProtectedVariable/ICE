//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Material.h"

namespace ICE {

    Shader *Material::getShader() const {
        return shader;
    }

    void Material::setShader(Shader *shader) {
        Material::shader = shader;
    }

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

    Material::Material(Shader *shader) : Material(shader, Eigen::Vector3f(1,1,1),Eigen::Vector3f(1,1,1),Eigen::Vector3f(1,1,1), 1) {}


    Material::Material(Shader *shader, const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                       const Eigen::Vector3f &ambient, float alpha) : Material(shader, albedo, specular, ambient, alpha,
                                                                               nullptr, nullptr, nullptr) {}

    Material::Material(Shader *shader, const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                       const Eigen::Vector3f &ambient, float alpha, const Texture *diffuseMap, const Texture *specularMap,
                       const Texture *ambientMap) : Material(shader, albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, nullptr){}

    Material::Material(Shader *shader, const Eigen::Vector3f &albedo, const Eigen::Vector3f &specular,
                       const Eigen::Vector3f &ambient, float alpha, const Texture *diffuseMap,
                       const Texture *specularMap, const Texture *ambientMap, const Texture *normalMap) : shader(shader), albedo(albedo),
                                                                                                          specular(specular), ambient(ambient),
                                                                                                          alpha(alpha), diffuseMap(diffuseMap), specularMap(specularMap),
                                                                                                          ambientMap(ambientMap), normalMap(normalMap) {}

    const Texture *Material::getDiffuseMap() const {
        return diffuseMap;
    }

    void Material::setDiffuseMap(const Texture *diffuseMap) {
        this->diffuseMap = diffuseMap;
    }

    const Texture *Material::getSpecularMap() const {
        return specularMap;
    }

    void Material::setSpecularMap(const Texture *specularMap) {
        this->specularMap = specularMap;
    }

    const Texture *Material::getAmbientMap() const {
        return ambientMap;
    }

    void Material::setAmbientMap(const Texture *ambientMap) {
        this->ambientMap = ambientMap;
    }

    const Texture *Material::getNormalMap() const {
        return normalMap;
    }

    void Material::setNormalMap(const Texture *normalMap) {
        this->normalMap = normalMap;
    }

}