//
// Created by Thomas Ibanez on 03.08.21.
//

#ifndef ICE_ASSETPATH_H
#define ICE_ASSETPATH_H

#include <optional>
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

    // Open the asset-type set: register a path prefix for a new asset type so WithTypePrefix<T> and
    // prefix-based project loading work for plugin-defined kinds. Built-in types are pre-registered.
    // Idempotent for an identical (type, prefix); throws (ICEException) on a conflicting prefix or a
    // second, different prefix for an already-registered type -- duplicate registration is rejected
    // loudly rather than silently shadowing. Not synchronized: call on the main thread only (matches
    // how loaders are registered, at load/plugin-init time).
    static void registerType(std::type_index type, const std::string& prefix);
    template<typename T>
    static void registerType(const std::string& prefix) {
        registerType(std::type_index(typeid(T)), prefix);
    }

    // Reverse of the prefix registration: the asset type a prefix maps to, or nullopt if unknown.
    // Used by project loading to route a persisted "prefix" section to the right erased loader.
    static std::optional<std::type_index> typeForPrefix(const std::string& prefix);

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
    static std::unordered_map<std::string, std::type_index> prefixes;  // reverse of typenames
};
}  // namespace ICE
namespace std {
template<>
struct hash<ICE::AssetPath> {
    std::size_t operator()(const ICE::AssetPath& k) const { return hash<string>()(k.toString()); }
};
}  // namespace std

#endif  //ICE_ASSETPATH_H
