#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/ViewportWidget.h"

class Viewport : public Controller {
   public:
    Viewport(const std::shared_ptr<ICE::ICEEngine> &engine, const std::function<void()> &entity_transformed_callback,
             const std::function<void(ICE::Entity e)> &entity_picked_callback);
    bool update() override;

    void setSelectedEntity(ICE::Entity e);

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    ViewportWidget ui;
    // World units per second; the per-frame step is camera_speed * dt so movement is
    // frame-rate independent. scroll_speed is world units per mouse-wheel notch.
    const double camera_speed = 6.0;
    const double scroll_speed = 0.5;
    ImGuizmo::OPERATION m_guizmo_mode = ImGuizmo::TRANSLATE;
    ICE::Entity m_selected_entity = 0;
    std::function<void()> m_entity_transformed_callback = [] {
    };
    std::function<void(ICE::Entity e)> m_entity_picked_callback = [](ICE::Entity) {
    };
    std::shared_ptr<ICE::Framebuffer> m_picking_frambuffer;
};
