//
// Created by Thomas Ibanez on 29.11.20.
//

#pragma once

#include <GraphicsFactory.h>
#include <Material.h>
#include <Mesh.h>
#include <Texture.h>

#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Asset.h"
#include "AssetLoader.h"
#include "AssetPath.h"
#include "Resource.h"

#define ICE_ASSET_PREFIX "__ice__"

namespace ICE {
class JobScheduler;             // async import runs staging on this pool (forward-declared: header stays light)
struct AssetBankAsyncState;     // shared worker<->main completion state (defined in AssetBank.cpp)

// Lifecycle state of an asset entry. Synchronous adds are Ready immediately; async requestAsset
// reservations start Loading and become Ready or Failed after a pump(). Resident is reserved for the
// GPU-residency policy (see the eviction task) and is not set by the async path yet.
enum class AssetState { Loading, Ready, Failed, Resident };

struct AssetBankEntry {
    AssetBankEntry() : path(""), asset(nullptr) {}
    AssetBankEntry(const AssetPath& _path, const std::shared_ptr<Asset>& _asset) : path(_path), asset(_asset) {}
    AssetPath path;
    std::shared_ptr<Asset> asset;
    AssetState state = AssetState::Ready;  // synchronous adds are usable at once
};

class AssetBank {
   public:
    // Async import phases (see requestAsset): `stage` runs on a worker and returns an opaque payload;
    // `commit` runs on the main thread in pump() and turns it into the final Asset.
    using StageFn = std::function<std::shared_ptr<void>()>;
    using CommitFn = std::function<std::shared_ptr<Asset>(const std::shared_ptr<void>&)>;

    AssetBank();
    ~AssetBank();

    // Registration seam for asset loaders. The concrete loaders live in the `io`
    // module; the composition layer wires them in via this method so that `assets`
    // does not depend on `io` (see registerDefaultLoaders in the io module).
    template<typename T>
    void addLoader(const std::shared_ptr<IAssetLoader<T>>& asset_loader) {
        loader.AddLoader<T>(asset_loader);
    }

    // Register a loader for a plugin-defined asset kind together with its path prefix in one call:
    // after this, WithTypePrefix<T> / addAsset<T> / getAsset<T> / project persistence all work for T
    // exactly as for a built-in type. Throws (via AssetPath::registerType) if the prefix collides
    // with an existing type. Built-in loaders keep using the prefix-less overload above (their
    // prefixes are pre-registered).
    template<typename T>
    void addLoader(const std::string& prefix, const std::shared_ptr<IAssetLoader<T>>& asset_loader) {
        AssetPath::registerType<T>(prefix);
        loader.AddLoader<T>(asset_loader);
    }

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
        // Reject a failed load (loaders now return nullptr on error) instead of inserting a
        // null asset that would later be dereferenced.
        if (asset == nullptr) {
            return false;
        }
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

    // Type-erased variant keyed by std::type_index: loads `sources` through the registered loader for
    // `type` and inserts it under `id`. Used by project loading to restore a persisted custom asset
    // whose concrete type is only known by its path prefix (see AssetPath::typeForPrefix). Returns
    // false on a UID/name collision or a null (failed) load; propagates ICEException if no loader is
    // registered for `type`.
    bool addAssetWithSpecificUID(std::type_index type, const AssetPath& name, const std::vector<std::filesystem::path>& sources, AssetUID id) {
        if (resources.find(id) == resources.end() && nameMapping.find(name) == nameMapping.end()) {
            auto res = loader.LoadResource(type, sources);
            if (!res) {
                return false;
            }
            resources.try_emplace(id, AssetBankEntry{name, res});
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
            resources.erase(id);
            // Notify listeners (e.g. the GPU registry) that this UID is gone so they can release any
            // resources keyed on it. Fires with just the UID -- never the erased entry -- so the
            // order relative to the erase above is irrelevant and re-import (remove + re-add under
            // the same UID) correctly drops the stale GPU upload.
            for (const auto& [handle, listener] : m_removal_listeners) {
                listener(id);
            }
            return true;
        }
        return false;
    }

    // Removal-listener registration seam. A listener is invoked (with the removed asset's UID) from
    // removeAsset. The GPU registry subscribes here so eviction happens without `assets` gaining a
    // reverse dependency on the graphics side. Returns a handle for later removeRemovalListener.
    using RemovalListenerHandle = std::size_t;
    RemovalListenerHandle addRemovalListener(std::function<void(AssetUID)> listener) {
        RemovalListenerHandle handle = m_next_listener_handle++;
        m_removal_listeners.emplace_back(handle, std::move(listener));
        return handle;
    }
    void removeRemovalListener(RemovalListenerHandle handle) {
        std::erase_if(m_removal_listeners, [handle](const auto& entry) { return entry.first == handle; });
    }

    // --- Async import ----------------------------------------------------------------------------
    // The scheduler used to stage async loads off the main thread. Null (the default) makes
    // requestAsset stage inline (still finalized by pump), so the synchronous behavior is preserved.
    // The bank co-owns the scheduler so it can drain in-flight loads at teardown.
    void setScheduler(const std::shared_ptr<JobScheduler>& scheduler);

    // Reserve `name`'s UID immediately (state Loading, null payload) so scenes/components can
    // reference it at once, then load it in the background: `stage` runs on a worker (it MUST NOT
    // touch the bank), `commit` runs on the main thread in pump() and produces the final Asset (it
    // may add sub-assets to the bank). getAsset returns nullptr until the load is Ready. Returns the
    // reserved UID, or the existing one if `name` is already requested/loaded (de-dup by path). This
    // is the path model import uses (stage = ModelLoader::stage, commit = ModelLoader::commit).
    template<typename T>
    AssetUID requestAsset(const std::string& name, StageFn stage, CommitFn commit) {
        return requestAssetImpl(AssetPath::WithTypePrefix<T>(name), std::move(stage), std::move(commit));
    }

    // Convenience for a pure-loader asset type (Mesh/Texture/Material/Shader/custom): stages by
    // running the registered loader on a worker and commits by simply publishing the result. NOT for
    // Model (its loader mutates the bank); use the stage/commit overload for that. Throws if no
    // loader is registered for T.
    template<typename T>
    AssetUID requestAsset(const std::string& name, const std::vector<std::filesystem::path>& sources) {
        auto loader_ptr = loader.getLoader(typeid(T));
        if (!loader_ptr) {
            throw ICEException("No matching loader for resource");
        }
        StageFn stage = [loader_ptr, sources]() -> std::shared_ptr<void> {
            return std::static_pointer_cast<void>(loader_ptr->loadErased(sources));
        };
        CommitFn commit = [](const std::shared_ptr<void>& staged) -> std::shared_ptr<Asset> {
            return std::static_pointer_cast<Asset>(staged);
        };
        return requestAssetImpl(AssetPath::WithTypePrefix<T>(name), std::move(stage), std::move(commit));
    }

    // Lifecycle state of `uid` (Failed for an unknown UID). Consumers poll this to know when an
    // async-requested asset is usable.
    AssetState getState(AssetUID uid) const;

    // Finalize completed async loads on the main thread: publish payloads, run model sub-asset
    // commits, and mark Ready/Failed. Call once per frame (ICEEngine::step). Cheap no-op when idle.
    void pump();

    // Block until every in-flight async load has been finalized, pumping while waiting. Used at
    // teardown and for load-then-wait startup. Requires the staging scheduler to still be alive
    // (the bank co-owns it via setScheduler), so it cannot stall.
    void flushAsync();

    // Number of async loads reserved but not yet finalized.
    std::size_t inFlight() const;

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
        // O(1): the entry already stores its path. This used to linear-scan nameMapping,
        // which made project saves (getName per asset) O(n^2).
        auto it = resources.find(uid);
        return it == resources.end() ? AssetPath("") : it->second.path;
    }

    AssetUID getUID(const AssetPath& name) {
        if (nameMapping.find(name) != nameMapping.end())
            return nameMapping[name];
        return NO_ASSET_ID;
    }

    bool nameInUse(const AssetPath& name);

   private:
    // Shared implementation of the requestAsset overloads: reserve the UID (main thread), then stage
    // on the scheduler (or inline if none) and record the commit for pump().
    AssetUID requestAssetImpl(const AssetPath& path, StageFn stage, CommitFn commit);

    AssetUID nextUID = 1;
    std::unordered_map<AssetPath, AssetUID> nameMapping;
    std::unordered_map<AssetUID, AssetBankEntry> resources;
    AssetLoader loader;
    std::vector<std::pair<RemovalListenerHandle, std::function<void(AssetUID)>>> m_removal_listeners;
    RemovalListenerHandle m_next_listener_handle = 1;

    // Async import state. m_pending_commits (main-thread only) holds each reservation's commit; the
    // shared m_async carries stage results from workers back to pump() and survives the bank if a
    // worker is still running -- workers never touch the bank directly. m_scheduler is co-owned so it
    // outlives in-flight loads through flushAsync().
    std::unordered_map<AssetUID, CommitFn> m_pending_commits;
    std::shared_ptr<AssetBankAsyncState> m_async;
    std::shared_ptr<JobScheduler> m_scheduler;
};
}  // namespace ICE
