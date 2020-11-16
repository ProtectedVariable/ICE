//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_SCENE_H
#define ICE_SCENE_H

#include <vector>
#include "Entity.h"
#include <string>

namespace ICE {
    class Scene {
    public:
        class SceneNode {
        public:
            std::vector<SceneNode*> children;
            Entity* entity;

            SceneNode(Entity* entity1) : children(std::vector<SceneNode*>()), entity(entity1) {};
        };

        Scene();

        bool addEntity(std::string parent, std::string uid, Entity &entity);
        bool renameEntity(std::string oldName,std::string newName);
        void setParent(std::string entity, std::string newParent);
        SceneNode* getByID(std::string uid);


    private:
        SceneNode root;
        std::unordered_map<std::string, SceneNode*> nodeByID;
    };
}


#endif //ICE_SCENE_H
