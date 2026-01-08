#pragma once

#include <ICEEngine.h>
#include <UI/MaterialEditDialog.h>

#include "AssetsRenderer.h"
#include "Controller.h"

class MaterialEditor : public Controller {
   public:
    MaterialEditor(const std::shared_ptr<ICE::ICEEngine> &engine);

    void open(const ICE::AssetPath &path);

    bool update() override;

   private:
    MaterialEditDialog ui;
    std::vector<ICE::AssetUID> m_shaders;
    bool m_is_open = false;
    bool m_done = false;
    std::shared_ptr<ICE::ICEEngine> m_engine;
    AssetsRenderer m_renderer;
    std::shared_ptr<ICE::Material> m_current_material;
    ICE::AssetPath material_path{""};
    float t = 0;
};
