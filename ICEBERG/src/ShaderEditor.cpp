#include "ShaderEditor.h"

#include "ShaderExporter.h"
#include "ShaderLoader.h"

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
    ui.setShader(m_current_shader, path.getName(), m_engine->getProject()->getBaseDirectory() / "Assets" / "Shaders");
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

            auto shader_root = m_engine->getProject()->getBaseDirectory() / "Assets" / "Shaders";
            if (!m_current_shader->getSources().empty()) {
                shader_root = m_current_shader->getSources().at(0).parent_path();
            }
            ICE::ShaderSource sources;
            for (const auto &[stage, widget] : ui.getStageWidgets()) {
                std::ofstream outstream;
                outstream.open(shader_root / widget->getShaderFile());
                outstream << widget->getShaderSource();
                outstream.close();
                sources.try_emplace(stage, std::make_pair(widget->getShaderFile(), widget->getShaderSource()));
            }
            ICE::Shader tmp_shader{sources};
            ICE::ShaderExporter sh;
            sh.writeToJson(shader_root / (ui.getName() + ".shader.json"), tmp_shader);

            auto new_shader = ICE::ShaderLoader().load({shader_root / (ui.getName() + ".shader.json")});
            if (m_shader_path.toString().empty()) {
                m_shader_path = ICE::AssetPath::WithTypePrefix<ICE::Shader>(ui.getName());
                m_engine->getAssetBank()->addAsset(m_shader_path, new_shader);
            } else {
                ICE::AssetPath new_path = m_shader_path;
                new_path.setName(ui.getName());
                *m_engine->getAssetBank()->getAsset<ICE::Shader>(m_shader_path) = *new_shader;
                m_engine->getAssetBank()->renameAsset(m_shader_path, new_path);

                //TODO: Invalidate shader to reload
            }
        } else if (ui.getResult() == DialogResult::Cancel) {
            m_is_open = false;
            m_done = true;
        }
    }
    return m_done;
}
