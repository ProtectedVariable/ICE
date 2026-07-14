//
// Created by Thomas Ibanez on 16.11.20.
//

#pragma once

#include <AssetBank.h>
#include <Camera.h>
#include <Entity.h>
#include <EntityHandle.h>
#include <Registry.h>
#include <SceneGraph.h>

#include <string>
#include <vector>

namespace ICE {
class Renderer;

class Scene {
   public:
    Scene(const std::string &name);

    bool setAlias(Entity entity, const std::string &newName);
    std::string getAlias(Entity e) const;

    std::shared_ptr<SceneGraph> getGraph() const;

    std::string getName() const;
    void setName(const std::string &name);

    std::shared_ptr<Registry> getRegistry() const;

    // The scene owns its view camera (a PerspectiveCamera by default). camera() returns a fluent,
    // non-owning handle for setup -- scene.camera().setPosition(p).pitch(-30) -- available as soon
    // as the scene exists, no getSystem<RenderSystem>() chain. When the scene is activated by the
    // engine, its render system is pointed at this camera. Replace it wholesale with setCamera();
    // cameraPtr() hands out the owning pointer (e.g. for the render system).
    CameraHandle camera() const;
    void setCamera(const std::shared_ptr<Camera> &camera);
    std::shared_ptr<Camera> cameraPtr() const;

    Entity createEntity();

    // Create an entity and return an ergonomic handle to it (optionally aliased). Prefer this
    // over createEntity() + registry gymnastics when writing gameplay/tools code.
    EntityHandle create(const std::string &name = "");
    // Wrap an existing entity id in a handle bound to this scene's registry.
    EntityHandle wrap(Entity e) const;

    Entity spawnTree(AssetUID model_id, const std::shared_ptr<AssetBank> &bank);

    // Instantiate a model's node/mesh hierarchy into this scene and return a handle to its root.
    // Uses the scene's asset bank (injected when the scene is added to a project), so gameplay
    // code just passes the model id: scene.spawn(project.importModel(...)).
    EntityHandle spawn(AssetUID model_id);

    // Asset bank backing spawn() and other by-name lookups. Set by Project when the scene is
    // added; may be null for a scene constructed standalone (spawn() then no-ops to NULL_ENTITY).
    void setAssetBank(const std::shared_ptr<AssetBank> &bank);

    void addEntity(Entity e, const std::string &alias, Entity parent);
    void removeEntity(Entity e);
    bool hasEntity(Entity e) const;

   private:
    std::string name;
    std::shared_ptr<SceneGraph> m_graph;
    std::unordered_map<Entity, std::string> aliases;
    std::shared_ptr<Registry> registry;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<AssetBank> m_asset_bank;
};
}  // namespace ICE
