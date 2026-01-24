#pragma once

#include <Registry.h>
#include <imgui.h>

#include "Components/ComboBox.h"
#include "Dialog.h"

class AddComponentPopup : public Dialog {
   public:
    AddComponentPopup() : m_components_combo("##add_component_combo", {"Render", "Light", "Animation"}) {}

    void setData(const std::shared_ptr<ICE::Registry> &registry, ICE::Entity entity) {
        m_registry = registry;
        m_entity = entity;
        
        std::vector<std::string> values;
        if (!registry->entityHasComponent<ICE::RenderComponent>(entity)) {
            values.push_back("Render Component");
        }
        if (!registry->entityHasComponent<ICE::LightComponent>(entity)) {
            values.push_back("Light Component");
        }
        if (!registry->entityHasComponent<ICE::AnimationComponent>(entity)) {
            values.push_back("Animation Component");
        }
        m_components_combo.setValues(values);
    }

    void render() {
        ImGui::PushID("add_component_popup");
        if (isOpenRequested()) {
            ImGui::OpenPopup("Add Component");
        }
        ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, FLT_MAX));
        if (ImGui::BeginPopupModal("Add Component", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            m_components_combo.render();
            if (ImGui::Button("Add")) {
                if (m_components_combo.getSelectedItem() == "Render Component")
                    m_registry->addComponent(m_entity, ICE::RenderComponent(0, 0));

                if (m_components_combo.getSelectedItem() == "Light Component")
                    m_registry->addComponent(m_entity, ICE::LightComponent(ICE::PointLight, Eigen::Vector3f(1, 1, 1)));

                if (m_components_combo.getSelectedItem() == "Animation Component")
                    m_registry->addComponent(m_entity, ICE::AnimationComponent{"", 0.0, 1.0, true, true});
                done(DialogResult::Ok);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                done(DialogResult::Cancel);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

   private:
    ComboBox m_components_combo;
    std::shared_ptr<ICE::Registry> m_registry;
    ICE::Entity m_entity;
};