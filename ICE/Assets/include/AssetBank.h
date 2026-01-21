//
// Created by Thomas Ibanez on 29.11.20.
//

#pragma once

#include <GraphicsFactory.h>
#include <Material.h>
#include <Mesh.h>
#include <Texture.h>

#include <memory>
#include <typeindex>
#include <unordered_map>

#include "Asset.h"
#include "AssetLoader.h"
#include "AssetPath.h"
#include "Resource.h"

#define ICE_ASSET_PREFIX "__ice__"

namespace ICE {
struct AssetBankEntry {
    AssetBankEntry() : path(""), asset(nullptr) {}
    AssetBankEntry(const AssetPath& _path, const std::shared_ptr<Asset>& _asset) : path(_path), asset(_asset) {}
    AssetPath path;
    std::shared_ptr<Asset> asset;
};

class AssetBank {
   public:
    AssetBank();

    template<typename T>
    std::shared_ptr<T> getAsset(AssetUID uid) {
        return dynamic_pointer_cast<T>(getAsset(uid));
    }

    template<typename T>
    std::shared_ptr<T> getAsset(const std::string& name) {
        return getAsset<T>(AssetPath::WithTypePrefix<T>(name));
    }

    template<typename T>
    std::shared_ptr<T> getAsset(const AssetPath& fullpath) {
        return getAsset<T>(getUID(fullpath));
    }

    std::shared_ptr<Asset> getAsset(AssetUID uid) {
        if (uid == NO_ASSET_ID || !resources.contains(uid))
            return nullptr;
        return resources[uid].asset;
    }

    template<typename T>
    bool addAsset(const std::string name, const std::vector<std::filesystem::path>& sources) {
        auto asset = loader.LoadResource<T>(sources);
        return addAsset<T>(name, asset);
    }

    template<typename T>
    bool addAsset(const std::string name, const std::shared_ptr<Asset>& asset) {
        return addAsset(AssetPath::WithTypePrefix<T>(name), asset);
    }

    bool addAsset(const AssetPath& path, const std::shared_ptr<Asset>& asset) {
        if (!nameMapping.contains(path) && !resources.contains(nextUID)) {
            resources.try_emplace(nextUID, AssetBankEntry{path, asset});
            nameMapping.try_emplace(path, nextUID);
            nextUID++;
            return true;
        }
        return false;
    }

    template<typename T>
    bool addAssetWithSpecificUID(const AssetPath& name, const std::vector<std::filesystem::path>& sources, AssetUID id) {
        if (resources.find(id) == resources.end() && nameMapping.find(name) == nameMapping.end()) {
            auto res = loader.LoadResource<T>(sources);
            resources[id] = AssetBankEntry(name, res);
            nameMapping[name] = id;
            nextUID = nextUID > id ? nextUID : id + 1;
            return true;
        }
        return false;
    }

    bool addAssetWithSpecificUID(const AssetPath& name, const std::shared_ptr<Asset>& asset, AssetUID id) {
        if (resources.find(id) == resources.end() && nameMapping.find(name) == nameMapping.end()) {
            resources.try_emplace(id, AssetBankEntry{name, asset});
            nameMapping.try_emplace(name, id);
            nextUID = nextUID > id ? nextUID : id + 1;
            return true;
        }
        return false;
    }

    bool renameAsset(const AssetPath& oldName, const AssetPath& newName) {
        if (oldName.toString() == newName.toString()) {
            return true;
        }
        if (oldName.prefix() != newName.prefix())
            return false;
        AssetUID id = nameMapping.find(oldName) == nameMapping.end() ? NO_ASSET_ID : nameMapping[oldName];
        if (id != NO_ASSET_ID) {
            if (nameMapping.find(newName) == nameMapping.end()) {
                nameMapping[newName] = id;
                resources[id].path = newName;
                nameMapping.erase(oldName);
                return true;
            }
        }
        return false;
    }

    bool removeAsset(const AssetPath& name) {
        if (nameMapping.find(name) != nameMapping.end()) {
            AssetUID id = getUID(name);
            nameMapping.erase(name);
            //TODO: Check resources[id].asset->unload();
            resources.erase(id);
            return true;
        }
        return false;
    }

    template<typename T>
    std::unordered_map<AssetUID, std::shared_ptr<T>> getAll() {
        std::unordered_map<AssetUID, std::shared_ptr<T>> all;
        for (const auto& [uid, entry] : resources) {
            auto asset = dynamic_pointer_cast<T>(entry.asset);
            if (asset != nullptr) {
                all.try_emplace(uid, asset);
            }
        }
        return all;
    }

    std::vector<AssetBankEntry> getAllEntries() {
        std::vector<AssetBankEntry> all;
        for (const auto& [uid, entry] : resources) {
            all.push_back(entry);
        }
        return all;
    }

    AssetPath getName(AssetUID uid) {
        for (const auto& [path, id] : nameMapping) {
            if (id == uid) {
                return path;
            }
        }
        return AssetPath("");
    }

    AssetUID getUID(const AssetPath& name) {
        if (nameMapping.find(name) != nameMapping.end())
            return nameMapping[name];
        return NO_ASSET_ID;
    }

    bool nameInUse(const AssetPath& name);

   private:
    AssetUID nextUID = 1;
    std::unordered_map<AssetPath, AssetUID> nameMapping;
    std::unordered_map<AssetUID, AssetBankEntry> resources;
    AssetLoader loader;
};
}  // namespace ICE
