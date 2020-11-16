//
// Created by Thomas Ibanez on 16.11.20.
//

#include "Scene.h"

namespace ICE {
    Scene::Scene() : root(SceneNode(nullptr)), nodeByID(std::unordered_map<std::string, SceneNode*>()) {
        nodeByID["root"] = &root;
    }

    bool Scene::addEntity(std::string parent, std::string uid, Entity &entity) {
        if(this->nodeByID.find(uid) != this->nodeByID.end()) {
            return false; //IDs must be unique
        }
        SceneNode* parentNode = this->getByID(parent);
        if(parentNode != nullptr) {
            SceneNode* childNode = new SceneNode(&entity);
            parentNode->children.push_back(childNode);
            nodeByID[uid] = childNode;
            return true;
        }
        return false;
    }

    bool Scene::renameEntity(std::string oldName, std::string newName) {
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

    void Scene::setParent(std::string entity, std::string newParent) {
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

    Scene::SceneNode *Scene::getByID(std::string uid) {
        if(this->nodeByID.find(uid) != this->nodeByID.end()) {
            return this->nodeByID[uid];
        }
        return nullptr;
    }
}