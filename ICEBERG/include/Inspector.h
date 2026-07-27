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
    // A component's Remove button fires while its widget is mid-render. Removing the component
    // there would free/null the pointer the widget is still using this frame, so the request is
    // recorded and applied after render() returns.
    enum class PendingRemove { None, Render, Light, Animation, AudioSource };

    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    InspectorWidget ui;
    ICE::Entity m_selected_entity = 0;
    int m_entity_has_changed = 0;
    PendingRemove m_pending_remove = PendingRemove::None;
    AddComponentPopup m_add_component_popup;
    // The inspector's clip audition voice. Kept so a second Preview click replaces the first
    // rather than layering sounds on top of each other.
    ICE::VoiceHandle m_preview_voice;
};
