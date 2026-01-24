#pragma once
#include <Entity.h>

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ICE {

class SceneGraph {
   public:
    struct SceneNode {
        Entity entity{0};
        std::vector<std::shared_ptr<SceneNode>> children{};

        // Use weak_ptr to avoid cyclic references (Parent owns Child, Child observes Parent)
        std::weak_ptr<SceneNode> parent;
    };

    SceneGraph() : root(std::make_shared<SceneNode>()) { idToNode.try_emplace(0, root); }

    void addEntity(Entity e) {
        // Prevent adding duplicate entities
        if (idToNode.find(e) != idToNode.end())
            return;

        auto sn = std::make_shared<SceneNode>();
        sn->entity = e;
        sn->parent = root;  // Default parent is root

        root->children.push_back(sn);
        idToNode[e] = sn;
    }

    void removeEntity(Entity e) {
        if (e == 0 || idToNode.find(e) == idToNode.end())
            return;

        auto sn = idToNode[e];
        auto parentPtr = sn->parent.lock();

        if (parentPtr) {
            // Remove 'sn' from its parent's list
            auto& siblings = parentPtr->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), sn), siblings.end());

            // Orphan handling: Reparent children to their grandparent
            for (auto& child : sn->children) {
                child->parent = parentPtr;  // Update child's parent pointer
                parentPtr->children.push_back(child);
            }
        }

        idToNode.erase(e);
    }

    void setParent(Entity e, Entity newParentID) {
        if (idToNode.find(e) == idToNode.end() || idToNode.find(newParentID) == idToNode.end())
            return;
        if (e == newParentID)
            return;  // Can't parent to self

        auto sn = idToNode[e];
        auto newParent = idToNode[newParentID];
        auto oldParent = sn->parent.lock();

        if (oldParent) {
            auto& siblings = oldParent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), sn), siblings.end());
        }

        sn->parent = newParent;
        newParent->children.push_back(sn);
    }

    Entity getParentID(Entity e) {
        if (idToNode.find(e) == idToNode.end())
            return 0;

        auto parentPtr = idToNode[e]->parent.lock();
        if (parentPtr) {
            return parentPtr->entity;
        }
        return 0;
    }

    const std::vector<Entity> getChildren(Entity e) {
        std::vector<Entity> childrenIDs;
        if (idToNode.find(e) != idToNode.end()) {
            for (auto& child : idToNode[e]->children) {
                childrenIDs.push_back(child->entity);
            }
        }
        return childrenIDs;
    }

    std::shared_ptr<SceneNode> getRoot() const { return root; }

   private:
    std::shared_ptr<SceneNode> root;
    std::unordered_map<Entity, std::shared_ptr<SceneNode>> idToNode;
};
}  // namespace ICE