//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

#include <Registry.h>

#include <utility>

namespace ICE {
Scene::Scene(const std::string &name) : name(name), m_graph(std::make_shared<SceneGraph>()), skybox(Skybox(NO_ASSET_ID)), registry(std::make_shared<Registry>()) {
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

const Skybox *Scene::getSkybox() const {
    return &skybox;
}

void Scene::setSkybox(const Skybox &skybox) {
    Scene::skybox = skybox;
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

void Scene::addEntity(Entity e, const std::string &alias, Entity parent) {
    m_graph->addEntity(e);
    m_graph->setParent(e, parent, false);
    aliases.try_emplace(e, alias);
}

void Scene::removeEntity(Entity e) {
    registry->removeEntity(e);
    aliases.erase(e);
    m_graph->removeEntity(e);
}
}  // namespace ICE