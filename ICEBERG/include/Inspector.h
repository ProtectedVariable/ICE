#pragma once

#include <Entity.h>
#include <ICEEngine.h>

#include "Controller.h"
#include "UI/AddComponentPopup.h"
#include "UI/InspectorWidget.h"

class Inspector : public Controller {
   public:
    Inspector(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;

    void setSelectedEntity(ICE::Entity e, bool force_refesh = false);
    bool entityHasChanged();

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    InspectorWidget ui;
    ICE::Entity m_selected_entity = 0;
    int m_entity_has_changed = 0;
    AddComponentPopup m_add_component_popup;
};
