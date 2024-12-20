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
        ImGui::PushID("add_component_popup");
        if (m_open) {
            ImGui::OpenPopup("Add Component");
            m_open = false;
        }
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::BeginPopupModal("Add Component", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
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
                m_accepted = true;
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    bool accepted() {
        if (m_accepted) {
            m_accepted = false;
            return true;
        }
        return false;
    }

   private:
    ComboBox m_components_combo;
    bool m_open = false;
    bool m_accepted = false;
    std::shared_ptr<ICE::Registry> m_registry;
    ICE::Entity m_entity;
};