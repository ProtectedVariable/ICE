#pragma once
#include <ImGUI/imgui.h>
#include <TransformComponent.h>

#include "Components/UniformInputs.h"
#include "Widget.h"

class InspectorWidget : public Widget {
   public:
    InspectorWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Inspector", 0, flags);

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
            for (auto& input : m_rc_inputs) {
                ImGui::Text("%s", input.getLabel().c_str());
                input.render();
            }
            ImGui::EndGroup();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void setTransformComponent(ICE::TransformComponent* tc) {
        m_tc = tc;
        m_tc_inputs.clear();
        m_tc_inputs.emplace_back("Position", tc->position);
        m_tc_inputs.back().onValueChanged([this](const ICE::UniformValue& v) { m_tc->position = std::get<Eigen::Vector3f>(v); });
        m_tc_inputs.emplace_back("Rotation", tc->rotation);
        m_tc_inputs.back().onValueChanged([this](const ICE::UniformValue& v) { m_tc->rotation = std::get<Eigen::Vector3f>(v); });
        m_tc_inputs.emplace_back("Scale", tc->scale);
        m_tc_inputs.back().onValueChanged([this](const ICE::UniformValue& v) { m_tc->scale = std::get<Eigen::Vector3f>(v); });
    }

    void setRenderComponent(ICE::RenderComponent* rc, const std::vector<std::string>& meshes_paths, const std::vector<ICE::AssetUID>& meshes_ids,
                            const std::vector<std::string>& materials_paths, const std::vector<ICE::AssetUID>& materials_ids) {
        m_rc = rc;
        m_rc_inputs.clear();
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

   private:
    ICE::TransformComponent* m_tc = nullptr;
    std::vector<UniformInputs> m_tc_inputs;

    ICE::RenderComponent* m_rc = nullptr;
    std::vector<UniformInputs> m_rc_inputs;
};
