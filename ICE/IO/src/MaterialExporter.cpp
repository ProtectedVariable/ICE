#include "MaterialExporter.h"

#include <json/json.h>
#include <fstream>
#include <variant>
#include <span>

using json = nlohmann::json;

namespace ICE {

void MaterialExporter::writeToJson(const std::filesystem::path &path, const Material &material) {
	std::ofstream outstream;
	outstream.open(path);
	json j;
	j["shader_id"] = material.getShader();
    j["transparent"] = material.isTransparent();

	std::vector<json> uniforms;
	for(const auto &[name, value] : material.getAllUniforms()) {
		json uniform;
		uniform["name"] = name;
		if (std::holds_alternative<float>(value)) {
			auto v = std::get<float>(value);
			uniform["type"] = "float";
			uniform["value"] = v;
		} else if (std::holds_alternative<int>(value)) {
			auto v = std::get<int>(value);
			uniform["type"] = "int";
			uniform["value"] = v;
		} else if (std::holds_alternative<AssetUID>(value)) {
			auto v = std::get<AssetUID>(value);
			uniform["type"] = "assetUID";
			uniform["value"] = v;
		} else if (std::holds_alternative<Eigen::Vector3f>(value)) {
			auto v = std::get<Eigen::Vector3f>(value);
			uniform["type"] = "vec3";
			uniform["value"] = {v.x(), v.y(), v.z()};
		} else if (std::holds_alternative<Eigen::Vector4f>(value)) {
			auto v = std::get<Eigen::Vector4f>(value);
			uniform["type"] = "vec4";
			uniform["value"] = {v.x(), v.y(), v.z(), v.w()};
		} else if (std::holds_alternative<Eigen::Matrix4f>(value)) {
			auto v = std::get<Eigen::Matrix4f>(value);
			uniform["type"] = "mat4";
			uniform["value"] = std::vector<float>(v.data(), v.data() + 16);
		} else {
			throw std::runtime_error("Uniform type not implemented");
		}
		uniforms.push_back(std::move(uniform));
	}

	j["uniforms"] = uniforms;
	outstream << j.dump(4);
	outstream.close();

}

void MaterialExporter::writeToBin(const std::filesystem::path &path, const Material &object){
	throw std::runtime_error("Not implemented");
}
}