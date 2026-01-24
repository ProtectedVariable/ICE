#pragma once

#include <ICEEngine.h>
#include <UI/ShaderEditDialog.h>

#include "AssetsRenderer.h"
#include "Controller.h"

class ShaderEditor : public Controller {
   public:
    ShaderEditor(const std::shared_ptr<ICE::ICEEngine> &engine);

    void open(const ICE::AssetPath &path);

    bool update() override;

   private:
    ShaderEditDialog ui;
    bool m_is_open = false;
    bool m_done = false;
    std::shared_ptr<ICE::ICEEngine> m_engine;
    std::shared_ptr<ICE::Shader> m_current_shader;
    ICE::AssetPath m_shader_path{""};
};
