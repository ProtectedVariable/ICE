//
// Created by Thomas Ibanez on 16.11.20.
//

#pragma once

#include <Entity.h>
#include <Registry.h>
#include <SceneGraph.h>
#include <Skybox.h>

#include <string>
#include <vector>

namespace ICE {
class Renderer;

class Scene {
   public:
    Scene(const std::string& name, Registry* registry);

    bool setAlias(Entity entity, const std::string& newName);
    std::string getAlias(Entity e);

    std::shared_ptr<SceneGraph> getGraph() const;

    std::string getName() const;
    void setName(const std::string& name);

    const Skybox* getSkybox() const;
    void setSkybox(const Skybox& skybox);

    Registry* getRegistry() const;
    Entity createEntity();
    void addEntity(Entity e, const std::string& alias, Entity parent);
    void removeEntity(Entity e);

   private:
    std::string name;
    Skybox skybox;
    std::shared_ptr<SceneGraph> m_graph;
    std::unordered_map<Entity, std::string> aliases;
    Registry* registry;
};
}  // namespace ICE
