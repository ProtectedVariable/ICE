//
// Created by Thomas Ibanez on 29.07.21.
//

#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace ICE {
class Resource {
   public:
    Resource(const std::vector<std::filesystem::path> &source) : source(source) {}
    std::vector<std::filesystem::path> getSources() const { return source; }
    void setSources(const std::vector<std::filesystem::path> &sources) { source = sources; }

   private:
    std::vector<std::filesystem::path> source;
};
}  // namespace ICE
