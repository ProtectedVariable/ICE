#pragma once

#include <ICEEngine.h>
#include <Scene.h>

#include "Controller.h"
#include "UI/HierarchyWidget.h"

class Hierarchy : public Controller {
   public:
    Hierarchy(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;
    SceneTreeView getTreeView(const std::shared_ptr<ICE::Scene> &scene) const;

    ICE::Entity getSelectedEntity() const;
    void setSelectedEntity(ICE::Entity e);
    void rebuildTree();

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    bool m_need_rebuild_tree = true;
    HierarchyWidget ui;
    ICE::Entity m_selected = 0;
};
