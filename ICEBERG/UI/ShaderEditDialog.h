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

    void setShader(const std::shared_ptr<ICE::Shader>& shader, const std::string& name) {
        m_widgets.clear();
        strncpy_s(m_shader_name, name.c_str(), 512);
        if (shader) {
            for (const auto& [stage, source] : shader->getStageSources()) {
                m_widgets.try_emplace(stage, std::make_unique<ShaderStageEditWidget>(stage));
                m_widgets[stage]->setShaderSource(source, shader->getSources().at(0).parent_path());
            }
        }
    }

    std::string getName() const { return std::string(m_shader_name); }
    std::unordered_map<ICE::ShaderStage, std::unique_ptr<ShaderStageEditWidget>> getStageWidgets() const { return m_widgets; }

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
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_apply") {
            ImGui::CloseCurrentPopup();
            done(DialogResult::Ok);
        } else if (node.arg<std::string>("id") == "btn_cancel") {
            ImGui::CloseCurrentPopup();
            done(DialogResult::Cancel);
        }
    }

   private:
    char m_shader_name[512] = {0};

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;

    std::unordered_map<ICE::ShaderStage, std::unique_ptr<ShaderStageEditWidget>> m_widgets;
};
