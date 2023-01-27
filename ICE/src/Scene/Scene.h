//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_SCENE_H
#define ICE_SCENE_H

#include <vector>
#include <ECS/Entity.h>
#include <ECS/Registry.h>
#include <string>
#include <Graphics/Renderer.h>
#include <Graphics/Skybox.h>

namespace ICE {
    class Renderer;

    class SceneGraph {
    public:
        struct SceneNode {
            Entity entity;
            std::vector<SceneNode*> children;
        };

        void addEntity(Entity e) {
            auto sn = createSceneNode(e);
            root.children.push_back(sn);
            idToNode.insert({e, sn});
        }

        void removeEntity(Entity e) {
            auto sn = idToNode[e];
            auto parent = findParent(e, &root);
            auto it = std::find(parent->children.begin(), parent->children.end(), idToNode[e]);
            parent->children.erase(it);
            for(size_t i = 0; i < sn->children.size(); i++) {
                parent->children.push_back(sn->children[i]);
            }
            sn->children.clear();
            idToNode.erase(e);
            free(sn);
        }

        void setParent(Entity e, Entity newParent, bool recurse) {
            auto parent = findParent(e, &root);
            auto sn = idToNode[e];
            auto newparent_node = idToNode[newParent];
            auto it = std::find(parent->children.begin(), parent->children.end(), sn);
            parent->children.erase(it);
            newparent_node->children.push_back(sn);
            if(!recurse) {
                for(size_t i = 0; i < sn->children.size(); i++) {
                    parent->children.push_back(sn->children[i]);
                }
            }
        }

        SceneGraph() : root(SceneNode{ .entity = 0, .children = std::vector<SceneNode*>()}), idToNode(std::unordered_map<Entity, SceneNode*>()) {}

        SceneNode root;
    private:
        SceneNode* findParent(Entity e, SceneNode* root) {
            auto it = std::find(root->children.begin(), root->children.end(), idToNode[e]);
            if (it != root->children.end()) {
                return root;
            }
            for(size_t i = 0; i < root->children.size(); i++) {
                SceneNode* subtreeSearch = findParent(e, root->children[i]);
                if(subtreeSearch != nullptr) {
                    return subtreeSearch;
                }
            }
            return nullptr;
        }

        SceneNode* createSceneNode(Entity e) {
            SceneNode* sn = new SceneNode();
            sn->entity = e;
            return sn;
        }

        std::unordered_map<Entity, SceneNode*> idToNode;
    };

    class Scene {
    public:
        
        Scene(const std::string& name, Registry* registry);

        bool setAlias(Entity entity, const std::string& newName);
        std::string getAlias(Entity e);

        SceneGraph getGraph();

        const std::string &getName() const;
        void setName(const std::string &name);

        Skybox* getSkybox();
        void setSkybox(const Skybox &skybox);

        Registry* getRegistry();
        Entity createEntity();
        void removeEntity(Entity e);
    private:
        std::string name;
        Skybox skybox;
        SceneGraph graph;
        std::unordered_map<Entity, std::string> aliases;
        Registry* registry;
    };
}


#endif //ICE_SCENE_H
