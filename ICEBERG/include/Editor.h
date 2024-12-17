#pragma once

#include <ICEEngine.h>
#include <UI/EditorWidget.h>
#include <UI/NewMaterialWidget.h>

#include <memory>
#include <vector>

#include "Assets.h"
#include "Controller.h"
#include "Hierarchy.h"
#include "Inspector.h"
#include "Viewport.h"

class Editor : public Controller {
   public:
    Editor(const std::shared_ptr<ICE::ICEEngine> &engine, const std::shared_ptr<ICE::GraphicsFactory> &g_factory);
    bool update() override;

   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    bool m_done = false;
    EditorWidget ui;
    std::unique_ptr<Viewport> m_viewport;
    std::unique_ptr<Hierarchy> m_hierarchy;
    std::unique_ptr<Inspector> m_inspector;
    std::unique_ptr<Assets> m_assets;
    ICE::Entity m_selected_entity;
    bool m_entity_transform_changed = false;

    //Popups
    NewMaterialWidget m_material_popup;
};
