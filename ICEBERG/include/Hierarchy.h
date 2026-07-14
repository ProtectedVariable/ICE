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

    // True once after the selected entity was renamed from the hierarchy. The Editor consumes
    // this to force-refresh the Inspector, whose name field is otherwise only reloaded on a
    // selection change (a rename keeps the same entity selected). Symmetric to
    // Inspector::entityHasChanged, which drives the hierarchy rebuild the other way.
    bool selectionRenamed();

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    bool m_need_rebuild_tree = true;
    bool m_selection_renamed = false;
    HierarchyWidget ui;
    ICE::Entity m_selected = 0;
};
