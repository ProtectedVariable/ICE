//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

#include <utility>

namespace ICE {
    Scene::Scene(const std::string& name) : name(name), graph(SceneGraph()), skybox(Skybox(NO_ASSET_ID)) {}

    Entity Scene::createEntity() {
        Entity e = entityManager.createEntity();
        addEntity(e);
        return e;
    }

    bool Scene::addEntity(Entity entity) {
        entities.push_back(entity);
        graph.addEntity(entity);
        aliases.insert({entity, "object"});
        return true;
    }

    std::vector<Entity> Scene::getEntities() const {
        return entities;
    }

    SceneGraph Scene::getGraph() {
        return graph;
    }

    const std::string &Scene::getName() const {
        return name;
    }

    void Scene::setName(const std::string &name) {
        Scene::name = name;
    }

    void Scene::removeEntity(Entity e) {
        auto it = std::find(entities.begin(), entities.end(), e);
        entities.erase(it);
        graph.removeEntity(e);
    }

    bool Scene::setAlias(Entity entity, const std::string& newName) {
        aliases[entity] = newName;
    }

    std::string Scene::getAlias(Entity e) {
        return aliases[e];
    }

    Skybox *Scene::getSkybox() {
        return &skybox;
    }

    void Scene::setSkybox(const Skybox &skybox) {
        Scene::skybox = skybox;
    }
}