#pragma once

namespace ICE {
class Registry;
class AssetBank;

// What a plugin is allowed to touch, handed to it at load time. Kept to non-owning pointers so the
// plugin depends on the ECS/asset seams, not on the concrete engine:
//   * registry   -- add gameplay systems (Registry::addSystem)
//   * asset_bank -- add asset loaders (AssetBank::addLoader)
// Components need no registration -- they register lazily on first use (see P2), so a plugin can
// define and use its own component types with no ceremony.
struct PluginContext {
    Registry* registry = nullptr;
    AssetBank* asset_bank = nullptr;
};

// Extension point: an out-of-tree module implements IPlugin to add systems, asset loaders and
// components to the engine without any change to core. The engine loads it via
// ICEEngine::loadPlugin, which builds a PluginContext for the active scene and calls registerWith.
class IPlugin {
   public:
    virtual ~IPlugin() = default;

    // Human-readable id (for logging / diagnostics).
    virtual const char* name() const = 0;

    // Register the plugin's systems / loaders / components. Called once at load.
    virtual void registerWith(PluginContext& ctx) = 0;
};

}  // namespace ICE
