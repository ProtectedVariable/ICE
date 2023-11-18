#pragma once

#include <ICEEngine.h>
#include <UI/EditorWidget.h>
#include <UI/NewMaterialWidget.h>

#include <memory>
#include <vector>

#include "Controller.h"

class Editor : public Controller {
   public:
    Editor(const std::shared_ptr<ICE::ICEEngine> &engine, const std::shared_ptr<ICE::GraphicsFactory> &g_factory);
    bool update() override;

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    EditorWidget ui;
    std::vector<std::unique_ptr<Controller>> m_subpannels;
    ICE::Entity m_selected_entity;

    //Popups
    NewMaterialWidget m_material_popup;
};
