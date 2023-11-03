#pragma once

#include <Entity.h>

#include <memory>

namespace ICE {

class SceneGraph {
   public:
    struct SceneNode {
        Entity entity{0};
        std::vector<std::shared_ptr<SceneNode>> children{};
    };

    SceneGraph() : root(std::make_shared<SceneNode>()) { idToNode.try_emplace(0, root); }

    void addEntity(Entity e) {
        auto sn = createSceneNode(e);
        root->children.push_back(sn);
        idToNode.insert({e, sn});
    }

    void removeEntity(Entity e) {
        auto sn = idToNode[e];
        auto parent = findParent(e, root);
        auto it = std::find(parent->children.begin(), parent->children.end(), idToNode[e]);
        parent->children.erase(it);
        for (size_t i = 0; i < sn->children.size(); i++) {
            parent->children.push_back(sn->children[i]);
        }
        sn->children.clear();
        idToNode.erase(e);
    }

    void setParent(Entity e, Entity newParent, bool recurse) {
        auto parent = findParent(e, root);
        auto sn = idToNode[e];
        auto newparent_node = idToNode[newParent];
        auto it = std::find(parent->children.begin(), parent->children.end(), sn);
        parent->children.erase(it);
        newparent_node->children.push_back(sn);
        if (!recurse) {
            for (size_t i = 0; i < sn->children.size(); i++) {
                parent->children.push_back(sn->children[i]);
            }
        }
    }

    Entity getParentID(Entity e) {
        auto it = std::find(root->children.begin(), root->children.end(), idToNode[e]);
        if (it != root->children.end()) {
            return root->entity;
        }
        for (size_t i = 0; i < root->children.size(); i++) {
            auto subtreeSearch = findParent(e, root->children[i]);
            if (subtreeSearch != nullptr) {
                return subtreeSearch->entity;
            }
        }
        return 0;
    }

    std::shared_ptr<SceneNode> root;

   private:
    std::shared_ptr<SceneNode> findParent(Entity e, const std::shared_ptr<SceneNode> &root) {
        auto it = std::find(root->children.begin(), root->children.end(), idToNode[e]);
        if (it != root->children.end()) {
            return root;
        }
        for (size_t i = 0; i < root->children.size(); i++) {
            auto subtreeSearch = findParent(e, root->children[i]);
            if (subtreeSearch != nullptr) {
                return subtreeSearch;
            }
        }
        return nullptr;
    }

    std::shared_ptr<SceneNode> createSceneNode(Entity e) {
        auto sn = std::make_shared<SceneNode>();
        sn->entity = e;
        return sn;
    }

    std::unordered_map<Entity, std::string> m_entity_names;
    std::unordered_map<Entity, std::shared_ptr<SceneNode>> idToNode;
};
}  // namespace ICE