//
// Created by Thomas Ibanez on 24.02.21.
//
#include <Material.h>
#include <gtest/gtest.h>

#include "MaterialExporter.h"

using namespace ICE;

TEST(MaterialExporterTest, EmptyMaterialOk) {
	Material m;
	MaterialExporter exporter;
	exporter.writeToJson("empty.json", m);
}

TEST(MaterialExporterTest, NonEmptyMaterialOk) {
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
}
