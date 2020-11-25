//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

#include <utility>

namespace ICE {
    Scene::Scene() : root(SceneNode(nullptr)), nodeByID(std::unordered_map<std::string, SceneNode*>()) {
        nodeByID["root"] = &root;
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

        if(this->nodeByID.find(newName) == this->nodeByID.end()) {
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
            it.second->children.erase(position);
        }
        this->nodeByID[newParent]->children.push_back(entityNode);
    }

    Scene::SceneNode *Scene::getByID(const std::string& uid) {
        if(this->nodeByID.find(uid) != this->nodeByID.end()) {
            return this->nodeByID[uid];
        }
        return nullptr;
    }

    std::vector<Entity *> Scene::getEntities() {
        auto nodes = std::vector<SceneNode*>();
        nodes.reserve(this->nodeByID.size());
        auto entities = std::vector<Entity*>();
        entities.reserve(this->nodeByID.size());
        for(const auto& kv : this->nodeByID) {
            if(kv.first != "root") {
                nodes.push_back(kv.second);
            }
        }
        for(auto n : nodes) {
            entities.push_back(n->entity);
        }
        return entities;
    }
}