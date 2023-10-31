#pragma once

#include <filesystem>

namespace ICE {
template<typename T>
class AssetExporter {
    virtual void writeToJson(const std::filesystem::path &path, const T &object) = 0;
    virtual void writeToBin(const std::filesystem::path &path, const T &object) = 0;
};
}  // namespace ICE
