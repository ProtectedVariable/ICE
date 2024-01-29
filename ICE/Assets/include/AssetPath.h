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
    AssetPath(const AssetPath& cpy);
    AssetPath(std::string path);
    std::string toString() const;
    std::string prefix() const;
    template<typename T>
    static AssetPath WithTypePrefix(std::string path) {
        return AssetPath(typenames[typeid(T)] + ASSET_PATH_SEPARATOR + path);
    }

    bool operator==(AssetPath other) const { return (other.toString() == this->toString()); }

    std::vector<std::string> getPath() const;

    std::string getName() const;

    void setName(const std::string& name);

   private:
    std::vector<std::string> path;
    std::string name;
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
