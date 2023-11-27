#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/AssetsWidget.h"
#include "UI/NewMaterialWidget.h"

class Assets : public Controller {
   public:
    Assets(const std::shared_ptr<ICE::ICEEngine> &engine, const std::shared_ptr<ICE::GraphicsFactory> &g_factory);
    bool update() override;

    void rebuildViewer();

   private:
    void *createThumbnail(const ICE::AssetBankEntry &entry);

    std::shared_ptr<ICE::ICEEngine> m_engine;
    std::shared_ptr<ICE::GraphicsFactory> m_g_factory;
    bool m_done = false;
    AssetsWidget ui;
    NewMaterialWidget m_material_widget;
};
