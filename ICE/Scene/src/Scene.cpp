//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

#include <Model.h>
#include <Registry.h>
#include <RenderComponent.h>
#include <RenderSystem.h>
#include <SceneCamera.h>
#include <SkeletonPoseComponent.h>
#include <SkinningComponent.h>
#include <TransformComponent.h>

#include <utility>

namespace ICE {
Scene::Scene(const std::string &name)
    : name(name),
      m_graph(std::make_shared<SceneGraph>()),
      registry(std::make_shared<Registry>()),
      // Free SceneCamera by default: identical behaviour to the scene-owned PerspectiveCamera it
      // replaces, but able to bind to a camera entity via setActiveCamera.
      m_camera(std::make_shared<SceneCamera>()) {
    aliases.try_emplace(0, "Scene");
}

std::shared_ptr<SceneGraph> Scene::getGraph() const {
    return m_graph;
}

std::string Scene::getName() const {
    return name;
}

void Scene::setName(const std::string &name) {
    Scene::name = name;
}

bool Scene::setAlias(Entity entity, const std::string &newName) {
    aliases[entity] = newName;
    return true;
}

std::string Scene::getAlias(Entity e) const {
    // find, not operator[]: the latter inserted an empty alias for unknown entities, which
    // (combined with the old alias-based hasEntity) made a nonexistent entity "exist".
    auto it = aliases.find(e);
    return it == aliases.end() ? std::string() : it->second;
}

std::shared_ptr<Registry> Scene::getRegistry() const {
    return registry;
}

CameraHandle Scene::camera() const {
    return CameraHandle(m_camera.get());
}

std::shared_ptr<Camera> Scene::cameraPtr() const {
    return m_camera;
}

void Scene::setCamera(const std::shared_ptr<Camera> &camera) {
    m_camera = camera;
    m_active_camera = NULL_ENTITY;  // an explicit camera object supersedes any active entity
    // If the scene is already active, re-point its render system at the new camera.
    if (auto rs = registry->tryGetSystem<RenderSystem>()) {
        rs->setCamera(camera);
    }
}

void Scene::setActiveCamera(Entity camera_entity) {
    m_active_camera = camera_entity;
    // Bind the existing SceneCamera in place where possible, so the shared_ptr the render system
    // already holds keeps pointing at the live view (no re-point needed). If setCamera() had
    // swapped in a non-SceneCamera, replace it with a bound one.
    auto scene_camera = std::dynamic_pointer_cast<SceneCamera>(m_camera);
    if (!scene_camera) {
        scene_camera = std::make_shared<SceneCamera>();
        m_camera = scene_camera;
    }
    scene_camera->bindEntity(camera_entity == NULL_ENTITY ? nullptr : registry.get(), camera_entity);

    if (auto rs = registry->tryGetSystem<RenderSystem>()) {
        rs->setCamera(m_camera);
    }
}

void Scene::setAssetBank(const std::shared_ptr<AssetBank> &bank) {
    m_asset_bank = bank;
}

EntityHandle Scene::spawn(AssetUID model_id) {
    if (!m_asset_bank) {
        return EntityHandle();  // no bank wired: nothing to spawn from
    }
    return wrap(spawnTree(model_id, m_asset_bank));
}

Entity Scene::createEntity() {
    Entity e = registry->createEntity();
    m_graph->addEntity(e);
    aliases.insert({e, "entity_" + std::to_string(e)});
    return e;
}

EntityHandle Scene::create(const std::string &name) {
    Entity e = createEntity();
    if (!name.empty()) {
        setAlias(e, name);
    }
    return EntityHandle(e, registry.get());
}

EntityHandle Scene::wrap(Entity e) const {
    return EntityHandle(e, registry.get());
}

Entity Scene::spawnTree(AssetUID model_id, const std::shared_ptr<AssetBank> &bank) {
    auto model = bank->getAsset<Model>(model_id);
    auto nodes = model->getNodes();
    auto meshes = model->getMeshes();
    auto materialIDs = model->getMaterialsIDs();
    auto useBones = !model->getSkeleton().boneMapping.empty();
    std::unordered_map<std::string, Entity> bone_entity;

    std::function<Entity(int, Entity, Entity)> spawnNode = [&](int nodeIndex, Entity parent, Entity skeleton) -> Entity {
        auto &node = nodes[nodeIndex];

        Entity nodeEntity = createEntity();
        if (skeleton == -1) {
            skeleton = nodeEntity;
        }
        setAlias(nodeEntity, node.name);
        if (model->getSkeleton().boneMapping.contains(node.name)) {
            bone_entity[node.name] = nodeEntity;
        }
        m_graph->setParent(nodeEntity, parent);
        registry->addComponent<TransformComponent>(nodeEntity, TransformComponent(node.localTransform));
        if (parent != 0) {
            auto parent_tc = registry->getComponent<TransformComponent>(parent);
            registry->getComponent<TransformComponent>(nodeEntity)->updateParentMatrix(parent_tc->getWorldMatrix());
        }
        for (int meshIdx : node.meshIndices) {
            Entity meshEntity = createEntity();
            setAlias(meshEntity, node.name + "_mesh_" + std::to_string(meshIdx));

            m_graph->setParent(meshEntity, nodeEntity);
            registry->addComponent<TransformComponent>(meshEntity, TransformComponent(Eigen::Matrix4f::Identity().eval()));
            registry->addComponent<RenderComponent>(meshEntity, RenderComponent(meshes[meshIdx], materialIDs[meshIdx]));
            if (useBones) {
                registry->addComponent<SkinningComponent>(meshEntity, SkinningComponent{.skeleton_entity = skeleton});
            }
        }

        for (int childIndex : node.children) {
            spawnNode(childIndex, nodeEntity, skeleton);
        }

        return nodeEntity;
    };

    auto root = spawnNode(0, 0, -1);

    if (useBones) {
        registry->addComponent<SkeletonPoseComponent>(root,
                                                      SkeletonPoseComponent{.skeletonModel = model_id,
                                                                            .bone_transform = std::vector<Eigen::Matrix4f>(
                                                                                model->getSkeleton().boneMapping.size(), Eigen::Matrix4f::Identity()),
                                                                            .bone_entity = bone_entity});
        auto pose = registry->getComponent<SkeletonPoseComponent>(root);
        for (const auto &[name, id] : model->getSkeleton().boneMapping) {
            Entity boneEntity = pose->bone_entity.at(name);

            Eigen::Matrix4f boneWorld = registry->getComponent<TransformComponent>(boneEntity)->getWorldMatrix();
            pose->bone_transform[id] = boneWorld;
        }
    }

    return root;
}

void Scene::addEntity(Entity e, const std::string &alias, Entity parent) {
    m_graph->addEntity(e);
    m_graph->setParent(e, parent);
    registry->addEntity(e);
    aliases.try_emplace(e, alias);
}

void Scene::removeEntity(Entity e) {
    if (e == NULL_ENTITY || !hasEntity(e)) {
        return;
    }
    // Removing an entity removes its whole subtree: a child's transform is expressed relative to
    // its parent, so orphans reparented onto the grandparent would silently jump in the world.
    // Collect the ids first -- the nodes are destroyed by removeSubtree below.
    for (Entity descendant : m_graph->collectSubtree(e)) {
        registry->removeEntity(descendant);
        aliases.erase(descendant);
    }
    m_graph->removeSubtree(e);
}

bool Scene::hasEntity(Entity e) const {
    // Aliveness is owned by the registry, not the alias map (an entity can be alive without
    // an alias, and getAlias no longer fabricates entries).
    return registry->isAlive(e);
}

}  // namespace ICE