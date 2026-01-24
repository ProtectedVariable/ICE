#include "MaterialEditor.h"

MaterialEditor::MaterialEditor(const std::shared_ptr<ICE::ICEEngine> &engine)
    : m_engine(engine),
      m_renderer(engine->getApi(), m_engine->getGraphicsFactory(), m_engine->getGPURegistry()) {
    ui.registerCallback("shader_selected", [this](int index) {
        if (index >= 0 && index < m_shaders.size())
            m_current_material->setShader(m_shaders[index]);
    });
}

void MaterialEditor::open(const ICE::AssetPath &path) {
    m_material_path = path;
    auto mtl = m_engine->getAssetBank()->getAsset<ICE::Material>(path);
    auto shaders = m_engine->getAssetBank()->getAll<ICE::Shader>();

    int selected_shader = 0;
    std::vector<std::string> shaders_paths;
    int i = 0;
    for (const auto &[id, ptr] : shaders) {
        m_shaders.push_back(id);
        shaders_paths.push_back(m_engine->getAssetBank()->getName(id).getName());
        if (mtl && mtl->getShader() == id) {
            selected_shader = i;
        }
        i++;
    }

    auto textures = m_engine->getAssetBank()->getAll<ICE::Texture2D>();
    std::unordered_map<std::string, ICE::AssetUID> textures_list;
    for (const auto &[id, ptr] : textures) {
        textures_list.try_emplace(m_engine->getAssetBank()->getName(id).toString(), id);
    }

    if (mtl) {
        m_current_material = std::make_shared<ICE::Material>(*mtl);
    } else {
        m_current_material = std::make_shared<ICE::Material>(false);
    }
    ui.setShaderList(shaders_paths, selected_shader);
    ui.setTextureList(textures_list);
    ui.setMaterialName(path.getName());
    ui.setMaterial(m_current_material);
    ui.open();
    m_is_open = true;
}

bool MaterialEditor::update() {
    m_done = false;
    if (m_is_open) {
        auto tex = m_renderer.getPreview(m_current_material, "mtl_editor_preview", t).first;
        ui.setPreviewTexture(tex);
        t += 0.1f;
        ui.render();
        if (ui.getResult() == DialogResult::Ok) {
            m_done = true;
            m_is_open = false;
            auto mtl = ui.getMaterial();
            mtl.removeUniform("");  //We use empty name as "delete me"

            if (m_material_path.toString().empty()) {
                m_material_path = ICE::AssetPath::WithTypePrefix<ICE::Material>(ui.getName());
                m_engine->getAssetBank()->addAsset(m_material_path, std::make_shared<ICE::Material>(mtl));
            } else {
                ICE::AssetPath new_path = m_material_path;
                new_path.setName(ui.getName());
                *m_engine->getAssetBank()->getAsset<ICE::Material>(m_material_path) = mtl;
                m_engine->getAssetBank()->renameAsset(m_material_path, new_path);
            }
        }
    }
    return m_done;
}
