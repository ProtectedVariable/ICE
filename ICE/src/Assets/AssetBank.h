//
// Created by Thomas Ibanez on 29.11.20.
//

#ifndef ICE_ASSETBANK_H
#define ICE_ASSETBANK_H

#include <unordered_map>
#include <Graphics/Mesh.h>
#include <Graphics/Material.h>
#include <Graphics/Texture.h>
#include "Asset.h"
#include "Resource.h"
#include "ResourceLoader.h"
#include "AssetPath.h"
#include <typeindex>

#define ICE_ASSET_PREFIX "__ice__"

namespace ICE {
    struct AssetBankEntry {
        std::string& name;
        Asset* asset;
    };

    class AssetBank {
    public:
        AssetBank();

        void fillWithDefaults();

        template<typename T>
        T *getAsset(AssetUID uid) {
            if(uid == NO_ASSET_ID) return nullptr;
            return dynamic_cast<T*>(getResource(uid)->asset);
        }

        template<typename T>
        T *getAsset(const std::string name) {
            return dynamic_cast<T*>(getResource(getUID(AssetPath::WithTypePrefix<T>(name)))->asset);
        }

        Resource *getResource(AssetUID uid) {
            if(uid == NO_ASSET_ID) return nullptr;
            return resources[uid];
        }

        Resource *getResource(const std::string name) {
            if(nameMapping.find(name) != nameMapping.end())
                return getResource(nameMapping[name]);
            return nullptr;
        }

        template<typename T>
        bool addResource(const std::string name, const std::vector<std::string> &sources) {
            Resource* res = loader.LoadResource<T>(sources);
            return addResource<T>(name, res);
        }

        template<typename T>
        bool addResource(const std::string name, Resource* res) {
            resources[nextUID] = res;
            nameMapping[AssetPath::WithTypePrefix<T>(name)] = nextUID;
            nextUID++;
            return true;
        }

        template<typename T>
        bool renameAsset(const std::string &oldName, const std::string &newName) {
            return true;
        }

        template<typename T>
        std::unordered_map<AssetUID, T*> getAll() {
            auto all = std::unordered_map<AssetUID, T*>();
            for(auto kv : resources) {
                T* asset = dynamic_cast<T*>(kv.second->asset);
                if(asset != nullptr) {
                    all[kv.first] = asset;
                }
            }
            return all;
        }

        AssetPath getName(AssetUID uid) {
            for(auto &entry : nameMapping) {
                if(entry.second == uid) {
                    return entry.first;
                }
            }
            return AssetPath("");
        }

        AssetUID getUID(AssetPath name) {
            if(nameMapping.find(name) != nameMapping.end())
                return nameMapping[name];
            return NO_ASSET_ID;
        }

        bool nameInUse(const std::string& name);
    private:
        AssetUID nextUID = 1;
        std::unordered_map<AssetPath, AssetUID> nameMapping;
        std::unordered_map<AssetUID, Resource*> resources;
        ResourceLoader loader;
    };
}


#endif //ICE_ASSETBANK_H
