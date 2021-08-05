//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

#include <utility>

namespace ICE {
    Scene::Scene(const std::string& name) : name(name), root(new SceneNode(nullptr)), nodeByID(std::unordered_map<std::string, SceneNode*>()), skybox(Skybox(NO_ASSET_ID)) {
        nodeByID["root"] = root;
    }

    bool Scene::addEntity(const std::string& parent, const std::string& uid, Entity* entity) {
        if(this->nodeByID.find(uid) != this->nodeByID.end()) {
            return false; //IDs must be unique
        }
        SceneNode* parentNode = this->getByID(parent);
        if(parentNode != nullptr) {
            auto* childNode = new SceneNode(entity);
            parentNode->children.push_back(childNode);
            nodeByID[uid] = childNode;
            return true;
        }
        return false;
    }

    bool Scene::renameEntity(const std::string& oldName, const std::string& newName) {
        if(this->nodeByID.find(newName) != this->nodeByID.end()) {
            return false;
        }

        if(this->nodeByID.find(oldName) == this->nodeByID.end()) {
            return false;
        }
        this->nodeByID[newName] = this->nodeByID[oldName];
        this->nodeByID.erase(oldName);
        return true;
    }

    void Scene::setParent(const std::string& entity, const std::string& newParent) {
        if(this->nodeByID.find(newParent) == this->nodeByID.end() || this->nodeByID.find(entity) == this->nodeByID.end()) {
            return;
        }
        SceneNode* entityNode = this->nodeByID[entity];
        for (auto &it: this->nodeByID) {
            auto position = std::find(it.second->children.begin(), it.second->children.end(), entityNode);
            if(position != it.second->children.end()) {
                it.second->children.erase(position);
                break;
            }
        }
        this->nodeByID[newParent]->children.push_back(entityNode);
    }

    Scene::SceneNode *Scene::getByID(const std::string& uid) {
        if(this->nodeByID.find(uid) != this->nodeByID.end()) {
            return this->nodeByID[uid];
        }
        return nullptr;
    }

    std::vector<Scene::SceneNode*> Scene::getNodes() const {
        auto nodes = std::vector<SceneNode*>();
        nodes.reserve(this->nodeByID.size());
        for(const auto& kv : this->nodeByID) {
            if(kv.first != "root") {
                nodes.push_back(kv.second);
            }
        }
        return nodes;
    }

    std::vector<Entity *> Scene::getEntities() const {
        auto nodes = getNodes();
        auto entities = std::vector<Entity*>();
        entities.reserve(this->nodeByID.size());
        for(auto n : nodes) {
            entities.push_back(n->entity);
        }
        return entities;
    }

    Scene::SceneNode* Scene::getRoot() {
        return nodeByID["root"];
    }

    const std::string Scene::idByNode(const Scene::SceneNode *node) {
        for(const auto& kv : this->nodeByID) {
            if(kv.second == node) {
                return kv.first;
            }
        }
        return std::string("");
    }

    const std::string Scene::idByEntity(const Entity* e) {
        if(e == nullptr) return std::string("root");
        for(const auto& kv : this->nodeByID) {
            if(kv.second->entity == e) {
                return kv.first;
            }
        }
        return std::string("");
    }

    bool Scene::addEntity(const std::string &uid, Entity *entity) {
        return addEntity("root", uid, entity);
    }

    const std::string &Scene::getName() const {
        return name;
    }

    void Scene::setName(const std::string &name) {
        Scene::name = name;
    }

    const std::string Scene::getParent(const std::string &child) {
        for(auto p : getNodes()) {
            for(auto c : p->children) {
                if(c == nodeByID[child]) {
                    return idByNode(p);
                }
            }
        }
        return "root";
    }

    void Scene::removeEntity(const std::string &uid) {
        SceneNode* toRemove = getByID(uid);
        if(toRemove == nullptr) return;
        nodeByID.erase(uid);
        auto allnodes = getNodes();
        allnodes.push_back(getByID("root"));
        for (auto p : allnodes) {
            int i = 0;
            for(auto c : p->children) {
                if(c == toRemove) {
                    p->children.erase(p->children.begin() + i);
                    return;
                }
                i++;
            }
        }
    }

    Skybox *Scene::getSkybox() {
        return &skybox;
    }

    void Scene::setSkybox(const Skybox &skybox) {
        Scene::skybox = skybox;
    }
}