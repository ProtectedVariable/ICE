#pragma once
#include <ForwardRenderer.h>
#include <Material.h>
#include <PerspectiveCamera.h>
#include <TransformComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <imgui.h>

#include <tuple>
#include <vector>

#include "Components/ComboBox.h"
#include "Components/InputText.h"
#include "Components/UniformInputs.h"
#include "Dialog.h"
#include "ShaderStageEditWidget.h"

class ShaderEditDialog : public Dialog, ImXML::XMLEventHandler {
   public:
    ShaderEditDialog() : m_xml_tree(ImXML::XMLReader().read("XML/ShaderEditDialog.xml")) {
        m_xml_renderer.addDynamicBind("str_shader_name", {m_shader_name, 512, ImXML::Chars});
    }

    void setShader(const std::shared_ptr<ICE::Shader>& shader, const std::string& name, const std::filesystem::path& shader_base_folder) {
        m_shader_base_folder = shader_base_folder;
        m_widgets.clear();
        strncpy_s(m_shader_name, name.c_str(), 512);
        if (shader) {
            for (const auto& [stage, source] : shader->getStageSources()) {
                m_shader_base_folder = shader->getSources().at(0).parent_path();
                m_widgets.try_emplace(stage, std::make_shared<ShaderStageEditWidget>(m_stage_names.at(stage)));
                m_widgets[stage]->setShaderSource(source, m_shader_base_folder);
            }
        }
    }

    std::string getName() const { return std::string(m_shader_name); }
    std::unordered_map<ICE::ShaderStage, std::shared_ptr<ShaderStageEditWidget>> getStageWidgets() const { return m_widgets; }

    void render() override {
        ImGui::PushID(m_dialog_id);
        if (isOpenRequested())
            ImGui::OpenPopup("Shader Editor");
        m_xml_renderer.render(m_xml_tree, *this);
        ImGui::PopID();
    }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "stages_widgets") {
            for (auto& [stage, widget] : m_widgets) {
                widget->render();
            }
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_add_stage") {
            if (ImGui::BeginPopup("add_stage_popup")) {
                if (ImGui::BeginCombo("###stages_combo", m_stage_names.at(m_selected_stage).c_str())) {
                    for (const auto& [stage, name] : m_stage_names) {
                        if (!m_widgets.contains(stage)) {
                            if (m_widgets.contains(m_selected_stage)) {
                                m_selected_stage = stage;
                            }
                            if (ImGui::Selectable(name.c_str(), stage == m_selected_stage)) {
                                m_selected_stage = stage;
                            }
                        }
                    }
                    ImGui::EndCombo();
                }
                if (ImGui::Button("Add")) {
                    m_widgets.try_emplace(m_selected_stage, std::make_shared<ShaderStageEditWidget>(m_stage_names.at(m_selected_stage)));
                    m_widgets.at(m_selected_stage)->setShaderSource({"", ""}, m_shader_base_folder);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_save") {
            done(DialogResult::Ok);
        } else if (node.arg<std::string>("id") == "btn_cancel") {
            done(DialogResult::Cancel);
        } else if (node.arg<std::string>("id") == "btn_add_stage") {
            ImGui::OpenPopup("add_stage_popup");
        }
    }

   private:
    char m_shader_name[512] = {0};
    ICE::ShaderStage m_selected_stage = ICE::ShaderStage::Vertex;

    std::filesystem::path m_shader_base_folder;
    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;

    std::unordered_map<ICE::ShaderStage, std::shared_ptr<ShaderStageEditWidget>> m_widgets;
    const std::unordered_map<ICE::ShaderStage, std::string> m_stage_names = {
        {ICE::ShaderStage::Vertex, "Vertex"},
        {ICE::ShaderStage::Fragment, "Fragment"},
        {ICE::ShaderStage::Geometry, "Geometry"},
        {ICE::ShaderStage::TessControl, "Tessellation Control"},
        {ICE::ShaderStage::TessEval, "Tessellation Evaluation"},
        {ICE::ShaderStage::Compute, "Compute"},
    };
};
