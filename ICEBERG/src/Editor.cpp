#include "Editor.h"

#include "Assets.h"
#include "Hierarchy.h"
#include "Inspector.h"
#include "Viewport.h"

Editor::Editor(const std::shared_ptr<ICE::ICEEngine>& engine, const std::shared_ptr<ICE::GraphicsFactory>& g_factory)
    : m_engine(engine),
      m_material_popup(engine) {
    m_subpannels.push_back(std::make_unique<Viewport>(engine));
    m_subpannels.push_back(std::make_unique<Hierarchy>(engine));
    m_subpannels.push_back(std::make_unique<Inspector>(engine));
    m_subpannels.push_back(std::make_unique<Assets>(engine, g_factory));
    ui.registerCallback("new_material_clicked", [this] {
        auto material = std::make_shared<ICE::Material>();
        auto path = ICE::AssetPath::WithTypePrefix<ICE::Material>("New Material");
        m_engine->getAssetBank()->addAsset(path, material);
        m_material_popup.open(m_engine->getAssetBank()->getUID(path));
    });
}

bool Editor::update() {
    ui.render();
    for (const auto& pannel : m_subpannels) {
        pannel->update();
        if (typeid(*pannel) == typeid(Hierarchy)) {
            m_selected_entity = static_cast<Hierarchy*>(pannel.get())->getSelectedEntity();
        } else if (typeid(*pannel) == typeid(Inspector)) {
            static_cast<Inspector*>(pannel.get())->setSelectedEntity(m_selected_entity);
        }
    }
    m_material_popup.render();
    return m_done;
}