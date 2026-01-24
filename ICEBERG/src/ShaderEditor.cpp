#include "ShaderEditor.h"

ShaderEditor::ShaderEditor(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {
}

void ShaderEditor::open(const ICE::AssetPath &path) {
    m_shader_path = path;
    auto shader = m_engine->getAssetBank()->getAsset<ICE::Shader>(path);

    if (shader) {
        m_current_shader = std::make_shared<ICE::Shader>(*shader);
    } else {
        m_current_shader = std::make_shared<ICE::Shader>();
    }
    ui.setShader(m_current_shader, path.getName());
    ui.open();
    m_is_open = true;
}

bool ShaderEditor::update() {
    m_done = false;
    if (m_is_open) {

        ui.render();
        if (ui.getResult() == DialogResult::Ok) {
            m_done = true;
            m_is_open = false;

            if (m_shader_path.toString().empty()) {
                m_shader_path = ICE::AssetPath::WithTypePrefix<ICE::Shader>(ui.getName());
                //m_engine->getAssetBank()->addAsset(m_shader_path, std::make_shared<ICE::Material>(mtl));
            } else {
                ICE::AssetPath new_path = m_shader_path;
                new_path.setName(ui.getName());
                //*m_engine->getAssetBank()->getAsset<ICE::Material>(m_shader_path) = mtl;
                m_engine->getAssetBank()->renameAsset(m_shader_path, new_path);
            }
        } else if (ui.getResult() == DialogResult::Cancel) {
            m_is_open = false;
            m_done = true;
        }
    }
    return m_done;
}
