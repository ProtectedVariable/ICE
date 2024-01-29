#pragma once

#include <Material.h>

#include "AssetExporter.h"

namespace ICE {
class MaterialExporter : public AssetExporter<Material> {
public:
    void writeToJson(const std::filesystem::path &path, const Material &object) override;
    void writeToBin(const std::filesystem::path &path, const Material &object) override;
};
}  // namespace ICE
