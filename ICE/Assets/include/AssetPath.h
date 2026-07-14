//
// Created by Thomas Ibanez on 03.08.21.
//

#ifndef ICE_ASSETPATH_H
#define ICE_ASSETPATH_H

#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#define ASSET_PATH_SEPARATOR ('/')

namespace ICE {
class AssetPath {
   public:
    AssetPath(const AssetPath& cpy) = default;  // copy members directly (no re-parse)
    AssetPath(std::string path);
    const std::string& toString() const;
    std::string prefix() const;
    template<typename T>
    static AssetPath WithTypePrefix(std::string path) {
        // .at() is read-only: operator[] inserted an empty prefix for unregistered types
        // (silently producing "/name" paths) and mutated the shared static map (data race).
        return AssetPath(typenames.at(typeid(T)) + ASSET_PATH_SEPARATOR + path);
    }

    // Compare the pre-computed canonical string (no allocation / re-parse per comparison).
    bool operator==(const AssetPath& other) const { return m_string == other.m_string; }

    std::vector<std::string> getPath() const;

    std::string getName() const;

    void setName(const std::string& name);

   private:
    std::vector<std::string> path;
    std::string name;
    std::string m_string;  // canonical "prefix/name", computed once in the constructor
    static std::unordered_map<std::type_index, std::string> typenames;
};
}  // namespace ICE
namespace std {
template<>
struct hash<ICE::AssetPath> {
    std::size_t operator()(const ICE::AssetPath& k) const { return hash<string>()(k.toString()); }
};
}  // namespace std

#endif  //ICE_ASSETPATH_H
