#pragma once
#include <imgui.h>

#include "AnimationComponentWidget.h"
#include "Components/InputText.h"
#include "Components/UniformInputs.h"
#include "LightComponentWidget.h"
#include "RenderComponentWidget.h"
#include "TransformComponentWidget.h"
#include "Widget.h"

class InspectorWidget : public Widget {
   public:
    InspectorWidget() {
        m_input_entity_name.onEdit([this](const std::string&, const std::string& text) { callback("entity_name_changed", text); });
    }

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        if (ImGui::Begin("Inspector", 0, flags)) {
            if (m_entity_selected) {
                ImGui::Text("Entity name");
                m_input_entity_name.render();

                m_tc_widget.render();
                m_rc_widget.render();
                m_lc_widget.render();
                m_ac_widget.render();

                if (ImGui::Button("Add Component...")) {
                    callback("add_component_clicked");
                }
            }
            ImGui::End();
        }
    }

    void setEntityName(const std::string& name) { m_input_entity_name.setText(name); }

    void setTransformComponent(ICE::TransformComponent* tc) {
        m_entity_selected = (tc != nullptr);
        m_tc_widget.setTransformComponent(tc);
    }
    void setLightComponent(ICE::LightComponent* lc) { m_lc_widget.setLightComponent(lc); }
    void setAnimationComponent(ICE::AnimationComponent* ac, const std::unordered_map<std::string, ICE::Animation>& animations) {
        m_ac_widget.setAnimationComponent(ac, animations);
    }
    void setRenderComponent(ICE::RenderComponent* rc, const std::vector<std::string>& meshes_paths, const std::vector<ICE::AssetUID>& meshes_ids,
                            const std::vector<std::string>& material_paths, const std::vector<ICE::AssetUID>& material_ids) {
        m_rc_widget.setRenderComponent(rc, meshes_paths, meshes_ids, material_paths, material_ids);
    }

   private:
    TransformComponentWidget m_tc_widget;
    RenderComponentWidget m_rc_widget;
    LightComponentWidget m_lc_widget;
    AnimationComponentWidget m_ac_widget;

    bool m_entity_selected = false;

    InputText m_input_entity_name{"##inspector_entity_name", ""};
};
