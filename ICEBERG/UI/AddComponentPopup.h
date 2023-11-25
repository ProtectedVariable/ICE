#pragma once

#include <ImGUI/imgui.h>
#include <Registry.h>

#include "Components/ComboBox.h"

class AddComponentPopup {
   public:
    AddComponentPopup() : m_components_combo("##add_component_combo", {"Render", "Transform", "Light"}) {}

    void open(const std::shared_ptr<ICE::Registry> &registry, ICE::Entity entity) {
        m_registry = registry;
        m_entity = entity;
        m_open = true;
    }

    void render() {
        if (m_open) {
            ImGui::OpenPopup("add_component_popup");
            m_open = false;
        }
        if (ImGui::BeginPopupModal("add_component_popup")) {
            m_components_combo.render();
            if (ImGui::Button("Add")) {
                switch (m_components_combo.getSelectedIndex()) {
                    case 0:
                        m_registry->addComponent(m_entity, ICE::RenderComponent(0, 0));
                        break;
                    case 1:
                        m_registry->addComponent(
                            m_entity, ICE::TransformComponent(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(1, 1, 1)));
                        break;
                    case 2:
                        m_registry->addComponent(m_entity, ICE::LightComponent(ICE::PointLight, Eigen::Vector3f(1, 1, 1)));
                        break;
                }
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

   private:
    ComboBox m_components_combo;
    bool m_open = false;
    std::shared_ptr<ICE::Registry> m_registry;
    ICE::Entity m_entity;
};