#pragma once

#include <ICEEngine.h>

#include "Controller.h"
#include "UI/AssetsBrowserWidget.h"
#include "UI/NewMaterialWidget.h"

class Assets : public Controller {
   public:
    Assets(const std::shared_ptr<ICE::ICEEngine> &engine, const std::shared_ptr<ICE::GraphicsFactory> &g_factory);
    bool update() override;

    void rebuildViewer();

   private:
    void createSubfolderView(AssetView *parent_view, const std::vector<std::string> &path, void *thumbnail);

    void *createThumbnail(const ICE::AssetBankEntry &entry);

    const std::vector<std::string> m_asset_categories = {"Models", "Materials", "Textures2D", "TextureCubes", "Shaders", "Others"};

    std::vector<AssetView> m_asset_views;
    int m_current_category_index = 0;

    std::shared_ptr<ICE::ICEEngine> m_engine;
    std::shared_ptr<ICE::GraphicsFactory> m_g_factory;
    bool m_done = false;
    AssetsBrowserWidget ui;
};
