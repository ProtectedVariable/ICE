#pragma once

#include <ImGUI/imgui.h>
#include <Registry.h>

#include "Components/ComboBox.h"

class AddComponentPopup {
   public:
    AddComponentPopup() : m_components_combo("##add_component_combo", {"Render", "Light"}) {}

    void open(const std::shared_ptr<ICE::Registry> &registry, ICE::Entity entity) {
        m_registry = registry;
        m_entity = entity;
        m_open = true;

        std::vector<std::string> values;
        if(!registry->entityHasComponent<ICE::RenderComponent>(entity)) {
            values.push_back("Render Component");
        }
        if(!registry->entityHasComponent<ICE::LightComponent>(entity)) {
            values.push_back("Light Component");
        }
        m_components_combo.setValues(values);
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
                if(m_components_combo.getSelectedItem() == "Render Component")
                    m_registry->addComponent(m_entity, ICE::RenderComponent(0));
                    
                if(m_components_combo.getSelectedItem() == "Light Component")
                    m_registry->addComponent(m_entity, ICE::LightComponent(ICE::PointLight, Eigen::Vector3f(1, 1, 1)));
                ImGui::CloseCurrentPopup();
                m_accepted = true;
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();

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