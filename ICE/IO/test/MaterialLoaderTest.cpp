//
// Created by Thomas Ibanez on 24.02.21.
//
#include <Material.h>
#include <Mesh.h>
#include <Shader.h>
#include <Texture.h>
#include <gtest/gtest.h>

#include "MaterialExporter.h"
#include "MaterialLoader.h"

using namespace ICE;

TEST(MaterialLoaderTest, LoadEmptyOk) {
    Material m;
    m.setShader(2023);
    m.setUniform("IntUniform1", 1);
    m.setUniform("FloatUniformPi", 3.14159f);
    m.setUniform("AssetUIDUniform16", 16ull);
    m.setUniform("Vector3fUniform", Eigen::Vector3f(0.5, 0.1, 10.2));
    m.setUniform("Vector4fUniform", Eigen::Vector4f(51, 26, 11.5, 0.30));
    Eigen::Matrix4f mat;
    mat << 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16;
    m.setUniform("Matrix4fUniform", mat);
    MaterialExporter exporter;
    exporter.writeToJson("filled.json", m);

    MaterialLoader loader;
    auto material_loaded = loader.load({"filled.json"});
    EXPECT_EQ(material_loaded->getShader(), m.getShader());
    for (const auto& [name, uniform] : m.getAllUniforms()) {
        if (std::holds_alternative<float>(uniform)) {
            auto v = std::get<float>(uniform);
            EXPECT_EQ(v, material_loaded->getUniform<float>(name));
        } else if (std::holds_alternative<int>(uniform)) {
            auto v = std::get<int>(uniform);
            EXPECT_EQ(v, material_loaded->getUniform<int>(name));
        } else if (std::holds_alternative<AssetUID>(uniform)) {
            auto v = std::get<AssetUID>(uniform);
            EXPECT_EQ(v, material_loaded->getUniform<AssetUID>(name));
        } else if (std::holds_alternative<Eigen::Vector3f>(uniform)) {
            auto v = std::get<Eigen::Vector3f>(uniform);
            EXPECT_EQ(v, material_loaded->getUniform<Eigen::Vector3f>(name));
        } else if (std::holds_alternative<Eigen::Vector4f>(uniform)) {
            auto v = std::get<Eigen::Vector4f>(uniform);
            EXPECT_EQ(v, material_loaded->getUniform<Eigen::Vector4f>(name));
        } else if (std::holds_alternative<Eigen::Matrix4f>(uniform)) {
            auto v = std::get<Eigen::Matrix4f>(uniform);
            EXPECT_EQ(v, material_loaded->getUniform<Eigen::Matrix4f>(name));
        }
    }
}
