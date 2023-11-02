//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

#include <Registry.h>

#include <utility>

namespace ICE {
Scene::Scene(const std::string &name, Registry *registry) : name(name), graph(SceneGraph()), skybox(Skybox(NO_ASSET_ID)), registry(registry) {
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

bool Scene::setAlias(Entity entity, const std::string &newName) {
    aliases[entity] = newName;
    return true;
}

std::string Scene::getAlias(Entity e) {
    return aliases[e];
}

const Skybox *Scene::getSkybox() const {
    return &skybox;
}

void Scene::setSkybox(const Skybox &skybox) {
    Scene::skybox = skybox;
}

Registry *Scene::getRegistry() const {
    return registry;
}

Entity Scene::createEntity() {
    Entity e = registry->createEntity();
    graph.addEntity(e);
    aliases.insert({e, "entity_" + std::to_string(e)});
    return e;
}

void Scene::removeEntity(Entity e) {
    registry->removeEntity(e);
    aliases.erase(e);
    graph.removeEntity(e);
}
}  // namespace ICE