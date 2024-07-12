#pragma once
#include <ImGUI/imgui.h>
#include <TransformComponent.h>

#include "Components/InputText.h"
#include "Components/UniformInputs.h"
#include "Widget.h"

class InspectorWidget : public Widget {
   public:
    InspectorWidget() {}

    void render() override {
        m_input_entity_name.onEdit([this](const std::string& text) { callback("entity_name_changed", text); });
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Inspector", 0, flags);

        ImGui::Text("Entity name");
        m_input_entity_name.render();

        if (m_tc) {
            ImGui::SeparatorText("Transform");
            ImGui::BeginGroup();
            for (auto& input : m_tc_inputs) {
                ImGui::Text("%s", input.getLabel().c_str());
                input.render();
            }
            ImGui::EndGroup();
        }
        if (m_rc) {
            ImGui::SeparatorText("Render");
            ImGui::BeginGroup();
            if (ImGui::Button("Remove")) {
                callback("remove_render_component_clicked");
            }
            for (auto& input : m_rc_inputs) {
                ImGui::Text("%s", input.getLabel().c_str());
                input.render();
            }
            ImGui::EndGroup();
        }
        if (m_lc) {
            ImGui::SeparatorText("Light");
            ImGui::BeginGroup();
            if (ImGui::Button("Remove")) {
                callback("remove_light_component_clicked");
            }
            for (auto& input : m_lc_inputs) {
                ImGui::Text("%s", input.getLabel().c_str());
                input.render();
            }
            ImGui::EndGroup();
        }

        if (ImGui::Button("Add Component...")) {
            callback("add_component_clicked");
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void setEntityName(const std::string& name) { m_input_entity_name.setText(name); }

    void setTransformComponent(ICE::TransformComponent* tc) {
        m_tc = tc;
        m_tc_inputs.clear();
        if (tc) {
            m_tc_inputs.emplace_back("Position", tc->getPosition());
            m_tc_inputs.back().onValueChanged([this](const ICE::UniformValue& v) { m_tc->setPosition(std::get<Eigen::Vector3f>(v)); });
            m_tc_inputs.emplace_back("Rotation", tc->getRotation());
            m_tc_inputs.back().onValueChanged([this](const ICE::UniformValue& v) { m_tc->setRotation(std::get<Eigen::Vector3f>(v)); });
            m_tc_inputs.emplace_back("Scale", tc->getScale());
            m_tc_inputs.back().onValueChanged([this](const ICE::UniformValue& v) { m_tc->setScale(std::get<Eigen::Vector3f>(v)); });
        }
    }

    void setRenderComponent(ICE::RenderComponent* rc, const std::vector<std::string>& meshes_paths, const std::vector<ICE::AssetUID>& meshes_ids,
                            const std::vector<std::string>& materials_paths, const std::vector<ICE::AssetUID>& materials_ids) {
        m_rc = rc;
        m_rc_inputs.clear();
        if (rc) {
            m_rc_inputs.reserve(2);
            UniformInputs in_mesh("Mesh", rc->mesh);
            in_mesh.onValueChanged([this](const ICE::UniformValue& v) { m_rc->mesh = std::get<ICE::AssetUID>(v); });
            in_mesh.setAssetComboList(meshes_paths, meshes_ids);
            m_rc_inputs.push_back(in_mesh);

            UniformInputs in_mat("Material", rc->material);
            in_mat.onValueChanged([this](const ICE::UniformValue& v) { m_rc->material = std::get<ICE::AssetUID>(v); });
            in_mat.setAssetComboList(materials_paths, materials_ids);
            m_rc_inputs.push_back(in_mat);
        }
    }

    void setLightComponent(ICE::LightComponent* lc) {
        m_lc = lc;
        m_lc_inputs.clear();
        if (lc) {
            m_lc_inputs.emplace_back("Color", m_lc->color);
            m_lc_inputs.back().onValueChanged([this](const ICE::UniformValue& v) { m_lc->color = std::get<Eigen::Vector3f>(v); });
        }
    }

   private:
    ICE::TransformComponent* m_tc = nullptr;
    std::vector<UniformInputs> m_tc_inputs;

    ICE::RenderComponent* m_rc = nullptr;
    std::vector<UniformInputs> m_rc_inputs;

    ICE::LightComponent* m_lc = nullptr;
    std::vector<UniformInputs> m_lc_inputs;

    InputText m_input_entity_name{"##inspector_entity_name", ""};
};
