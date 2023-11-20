#pragma once
#include <ImGUI/imgui.h>
#include <Material.h>
#include <TransformComponent.h>

#include <tuple>
#include <vector>

#include "Components/ComboBox.h"
#include "Components/InputText.h"
#include "Widget.h"

class NewMaterialWidget : public Widget {
   public:
    NewMaterialWidget() = default;

    void render() override {
        if (m_open) {
            ImGui::OpenPopup("Material Editor");
            m_open = false;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::BeginPopupModal("Material Editor", 0, 0)) {
            ImGui::Text("Name:");
            ImGui::SameLine();
            ImGui::InputText("##name", m_name, 512);
            ImGui::Text("Shader:");
            ImGui::SameLine();
            m_shaders_combo.render();
            ImGui::Text("Uniforms");
            ImGui::SameLine();
            if (ImGui::BeginTable("uniforms_table", 3)) {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Value");
                ImGui::TableSetupColumn("Type");
                ImGui::TableHeadersRow();
                for (int i = 0; i < m_uniforms.size(); i++) {
                    ImGui::TableNextColumn();
                    m_uniform_names[i].render();
                    ImGui::TableNextColumn();
                    ImGui::Text("todo");
                    ImGui::TableNextColumn();
                    m_uniform_combos[i].render();
                }
                ImGui::EndTable();
            }

            if (ImGui::Button("New Uniform")) {
                m_uniforms.emplace_back("uNew", 0, "Asset");
                m_uniform_names.emplace_back("##uName_" + std::to_string(m_ctr), "uNew");
                m_uniform_combos.emplace_back("##uType_" + std::to_string(m_ctr), uniform_types_names);
                m_ctr++;
            }

            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            ImGui::Button("Apply");
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void open() { m_open = true; }

    void setName(const std::string& name) { memcpy(m_name, name.c_str(), name.size() + 1); }
    void setShaders(const std::vector<std::string>& shader_names) { m_shaders_combo.setValues(shader_names); }
    void setUniforms(const std::vector<std::tuple<std::string, ICE::UniformValue, std::string>>& uniforms) {
        m_uniforms = uniforms;
        for (const auto& u : uniforms) {
            const auto& name = std::get<0>(u);
            m_uniform_names.emplace_back("##uName_" + std::to_string(m_ctr), name);
            m_uniform_combos.emplace_back("##uType_" + std::to_string(m_ctr), uniform_types_names);
            m_ctr++;
        }
    }

   private:
    bool m_open = false;
    char m_name[512] = {0};
    int m_shader_index = 0;
    ComboBox m_shaders_combo{"##shader_combo", {}};
    std::vector<InputText> m_uniform_names;
    std::vector<ComboBox> m_uniform_combos;
    std::vector<std::tuple<std::string, ICE::UniformValue, std::string>> m_uniforms;
    const std::vector<std::string> uniform_types_names = {"Asset", "Int", "Float", "Vector3", "Vector4", "Matrix4"};
    int m_ctr = 0;
};
