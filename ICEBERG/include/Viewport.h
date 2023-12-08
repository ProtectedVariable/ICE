#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/ViewportWidget.h"

class Viewport : public Controller {
   public:
    Viewport(const std::shared_ptr<ICE::ICEEngine> &engine);
    bool update() override;

    void setSelectedEntity(ICE::Entity e);

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    ViewportWidget ui;
    const double camera_delta = 0.1;
    ImGuizmo::OPERATION m_guizmo_mode = ImGuizmo::TRANSLATE;
    ICE::Entity m_selected_entity = 0;
};
