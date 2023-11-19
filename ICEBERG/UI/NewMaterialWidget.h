#pragma once
#include <ImGUI/imgui.h>
#include <Material.h>
#include <TransformComponent.h>

#include <tuple>
#include <vector>

#include "Widget.h"

class NewMaterialWidget : public Widget {
   public:
    NewMaterialWidget() = default;
    ~NewMaterialWidget() {
        for (auto& name : m_shader_names) {
            delete[] name;
        }
    }

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
            ImGui::Combo("##shader", &m_shader_index, m_shader_names.data(), m_shader_names.size(), 10);
            ImGui::Text("Uniforms");
            ImGui::SameLine();
            if (ImGui::BeginTable("uniforms_table", 3)) {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Value");
                ImGui::TableSetupColumn("Type");
                ImGui::TableHeadersRow();
                for (const auto& uniform : m_uniforms) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::get<0>(uniform).c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("todo");
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::get<2>(uniform).c_str());
                }
                ImGui::EndTable();
            }

            if (ImGui::Button("New Uniform")) {
                m_uniforms.emplace_back("uNew", 0, "int");
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
    void setShaders(const std::vector<std::string>& shader_names) {
        for (auto& name : m_shader_names) {
            delete[] name;
        }
        m_shader_names.resize(shader_names.size());
        std::transform(shader_names.begin(), shader_names.end(), m_shader_names.begin(), [](const std::string& name) {
            auto c_name = new char[512];
            memcpy(c_name, name.c_str(), name.size() + 1);
            return c_name;
        });
    }
    void setUniforms(const std::vector<std::tuple<std::string, ICE::UniformValue, std::string>>& uniforms) { m_uniforms = uniforms; }

   private:
    bool m_open = false;
    char m_name[512] = {0};
    int m_shader_index = 0;
    std::vector<const char*> m_shader_names;
    std::vector<std::tuple<std::string, ICE::UniformValue, std::string>> m_uniforms;
};
