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
        T *getAsset(const std::string& name) {
            return getAsset<T>(AssetPath::WithTypePrefix<T>(name));
        }

        template<typename T>
        T *getAsset(const AssetPath& fullpath) {
            Resource* res = getResource(getUID(fullpath));
            if(res != nullptr) {
                return dynamic_cast<T*>(res->asset);
            }
            return nullptr;
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
            return addResource(AssetPath::WithTypePrefix<T>(name), res);
        }

        bool addResource(const AssetPath& name, Resource* res) {
            resources[nextUID] = res;
            nameMapping[name] = nextUID;
            nextUID++;
            return true;
        }

        template<typename T>
        bool addResourceWithSpecificUID(const AssetPath& name, const std::vector<std::string> &sources, AssetUID id) {
            if(resources.find(id) == resources.end() && nameMapping.find(name) == nameMapping.end()) {
                Resource* res = loader.LoadResource<T>(sources);
                resources[id] = res;
                nameMapping[name] = id;
                nextUID = nextUID > id ? nextUID : id;
                return true;
            }
            return false;
        }

        bool renameAsset(const AssetPath &oldName, const AssetPath &newName) {
            if(oldName.getPath()[0] != newName.getPath()[0]) return false;
            AssetUID id = nameMapping.find(oldName) == nameMapping.end() ? 0 : nameMapping[oldName];
            if(id != NO_ASSET_ID) {
                if(nameMapping.find(newName) == nameMapping.end()) {
                    nameMapping[newName] = id;
                    nameMapping.erase(oldName);
                    return true;
                }
            }
            return false;
        }

        bool removeAsset(const AssetPath& name) {
            if(nameMapping.find(name) != nameMapping.end()) {
                AssetUID id = getUID(name);
                nameMapping.erase(name);
                resources.erase(id);
                //TODO: Actual deload and release of resources
                return true;
            }
            return false;
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

        bool nameInUse(const AssetPath& name);
    private:
        AssetUID nextUID = 1;
        std::unordered_map<AssetPath, AssetUID> nameMapping;
        std::unordered_map<AssetUID, Resource*> resources;
        ResourceLoader loader;
    };
}


#endif //ICE_ASSETBANK_H
