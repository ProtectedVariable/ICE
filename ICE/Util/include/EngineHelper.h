#pragma once

#include <ICEEngine.h>
#include <Registry.h>
#include <AssetBank.h>
#include <Scene.h>
#include <Project.h>
#include <Camera.h>
#include <RenderSystem.h>

#include <memory>

namespace ICE {

/**
 * @brief Helper class to simplify common ICE engine operations
 * 
 * Provides static utility functions to reduce the verbosity of
 * accessing commonly used engine components.
 * 
 * Example:
 *   auto reg = EngineHelper::getRegistry(engine);
 *   // vs
 *   auto reg = engine.getProject()->getCurrentScene()->getRegistry();
 */
class EngineHelper {
public:
    // Registry access
    static std::shared_ptr<Registry> getRegistry(ICEEngine& engine) {
        auto project = engine.getProject();
        if (!project) return nullptr;
        
        auto scene = project->getCurrentScene();
        if (!scene) return nullptr;
        
        return scene->getRegistry();
    }

    // AssetBank access
    static std::shared_ptr<AssetBank> getAssetBank(ICEEngine& engine) {
        auto project = engine.getProject();
        if (!project) return nullptr;
        return project->getAssetBank();
    }

    // Scene access
    static std::shared_ptr<Scene> getCurrentScene(ICEEngine& engine) {
        auto project = engine.getProject();
        if (!project) return nullptr;
        return project->getCurrentScene();
    }

    // Camera access
    static std::shared_ptr<Camera> getCamera(ICEEngine& engine) {
        auto reg = getRegistry(engine);
        if (!reg) return nullptr;
        
        auto renderSystem = reg->getSystem<RenderSystem>();
        if (!renderSystem) return nullptr;
        
        return renderSystem->getCamera();
    }

    // Entity creation helpers
    static Entity createEntity(ICEEngine& engine) {
        auto scene = getCurrentScene(engine);
        if (!scene) return 0;
        return scene->createEntity();
    }

    static Entity spawnModel(ICEEngine& engine, const std::string& modelName) {
        auto scene = getCurrentScene(engine);
        auto bank = getAssetBank(engine);
        if (!scene || !bank) return 0;

        auto modelId = bank->getUID(AssetPath::WithTypePrefix<Model>(modelName));
        if (modelId == NO_ASSET_ID) return 0;

        return scene->spawnTree(modelId, bank);
    }

    // Asset loading helpers
    static AssetUID getModelUID(ICEEngine& engine, const std::string& name) {
        auto bank = getAssetBank(engine);
        if (!bank) return NO_ASSET_ID;
        return bank->getUID(AssetPath::WithTypePrefix<Model>(name));
    }

    static AssetUID getTextureUID(ICEEngine& engine, const std::string& name) {
        auto bank = getAssetBank(engine);
        if (!bank) return NO_ASSET_ID;
        return bank->getUID(AssetPath::WithTypePrefix<Texture2D>(name));
    }

    static AssetUID getMaterialUID(ICEEngine& engine, const std::string& name) {
        auto bank = getAssetBank(engine);
        if (!bank) return NO_ASSET_ID;
        return bank->getUID(AssetPath::WithTypePrefix<Material>(name));
    }

    // System access
    template<typename T>
    static std::shared_ptr<T> getSystem(ICEEngine& engine) {
        auto reg = getRegistry(engine);
        if (!reg) return nullptr;
        return reg->getSystem<T>();
    }

    // Validation helpers
    static bool isEngineReady(ICEEngine& engine) {
        return engine.getProject() != nullptr 
            && engine.getProject()->getCurrentScene() != nullptr;
    }

    static bool hasAsset(ICEEngine& engine, const std::string& name) {
        auto bank = getAssetBank(engine);
        if (!bank) return false;
        // Check if any asset with this name exists
        return getModelUID(engine, name) != NO_ASSET_ID ||
               getTextureUID(engine, name) != NO_ASSET_ID ||
               getMaterialUID(engine, name) != NO_ASSET_ID;
    }
};

}  // namespace ICE
