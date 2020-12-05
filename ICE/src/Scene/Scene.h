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
    class Renderer;

    class Scene {
    public:
        class SceneNode {
        public:
            std::vector<SceneNode*> children;
            Entity* entity;

            SceneNode(Entity* entity1) : children(std::vector<SceneNode*>()), entity(entity1) {};
        };

        Scene();

        bool addEntity(const std::string& parent, const std::string& uid, Entity* entity);
        bool renameEntity(const std::string& oldName,const std::string& newName);
        void setParent(const std::string& entity, const std::string& newParent);

        std::vector<Entity*> getEntities() const;

        SceneNode* getByID(const std::string& uid);
        SceneNode* getRoot();
        const std::string& idByNode(const SceneNode* node);
        const std::string& idByEntity(const Entity* e);
    private:
        SceneNode* root;
        std::unordered_map<std::string, SceneNode*> nodeByID;
    };
}


#endif //ICE_SCENE_H
