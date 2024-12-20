//
// Created by Thomas Ibanez on 16.11.20.
//

#pragma once

#include <Entity.h>
#include <Registry.h>
#include <SceneGraph.h>

#include <string>
#include <vector>

namespace ICE {
class Renderer;

class Scene {
   public:
    Scene(const std::string& name);

    bool setAlias(Entity entity, const std::string& newName);
    std::string getAlias(Entity e);

    std::shared_ptr<SceneGraph> getGraph() const;

    std::string getName() const;
    void setName(const std::string& name);

    std::shared_ptr<Registry> getRegistry() const;
    Entity createEntity();
    void addEntity(Entity e, const std::string& alias, Entity parent);
    void removeEntity(Entity e);
    bool hasEntity(Entity e);

   private:
    std::string name;
    std::shared_ptr<SceneGraph> m_graph;
    std::unordered_map<Entity, std::string> aliases;
    std::shared_ptr<Registry> registry;
};
}  // namespace ICE
