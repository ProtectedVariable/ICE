//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

#include <Registry.h>

#include <utility>

namespace ICE {
Scene::Scene(const std::string &name) : name(name), m_graph(std::make_shared<SceneGraph>()), registry(std::make_shared<Registry>()) {
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

std::string Scene::getAlias(Entity e) {
    return aliases[e];
}

std::shared_ptr<Registry> Scene::getRegistry() const {
    return registry;
}

Entity Scene::createEntity() {
    Entity e = registry->createEntity();
    m_graph->addEntity(e);
    aliases.insert({e, "entity_" + std::to_string(e)});
    return e;
}

Entity Scene::spawnTree(AssetUID model_id, const std::shared_ptr<AssetBank> &bank) {
    auto model = bank->getAsset<Model>(model_id);
    auto nodes = model->getNodes();
    auto meshes = model->getMeshes();
    auto materialIDs = model->getMaterialsIDs();
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
            registry->addComponent<SkinningComponent>(meshEntity, SkinningComponent{.skeleton_entity = skeleton});
            registry->addComponent<RenderComponent>(meshEntity, RenderComponent(meshes[meshIdx], materialIDs[meshIdx]));
        }

        for (int childIndex : node.children) {
            spawnNode(childIndex, nodeEntity, skeleton);
        }

        return nodeEntity;
    };

    auto root = spawnNode(0, 0, -1);

    if (!model->getSkeleton().boneMapping.empty()) {
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
    registry->removeEntity(e);
    aliases.erase(e);
    m_graph->removeEntity(e);
}

bool Scene::hasEntity(Entity e) {
    return aliases.contains(e);
}

}  // namespace ICE