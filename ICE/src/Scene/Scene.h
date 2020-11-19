//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_SCENE_H
#define ICE_SCENE_H

#include <vector>
#include "Entity.h"
#include <string>
#include <Graphics/Renderer.h>

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

        const Renderer &getRenderer() const;

        void setRenderer(const Renderer &renderer);

    private:
        SceneNode root;
        std::unordered_map<std::string, SceneNode*> nodeByID;
        Renderer renderer;
    };
}


#endif //ICE_SCENE_H
