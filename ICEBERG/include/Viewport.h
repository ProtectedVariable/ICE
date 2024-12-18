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
    const double camera_delta = 0.1;
    ImGuizmo::OPERATION m_guizmo_mode = ImGuizmo::TRANSLATE;
    ICE::Entity m_selected_entity = 0;
    std::function<void()> m_entity_transformed_callback = [] {
    };
    std::function<void(ICE::Entity e)> m_entity_picked_callback = [](ICE::Entity) {
    };
    std::shared_ptr<ICE::Framebuffer> m_picking_frambuffer;
};
