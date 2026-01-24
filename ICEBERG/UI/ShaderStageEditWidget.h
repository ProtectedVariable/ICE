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
#include "Widget.h"

class ShaderStageEditWidget : public Widget, ImXML::XMLEventHandler {
   public:
    ShaderStageEditWidget(const std::string stage) : m_stage(stage), m_xml_tree(ImXML::XMLReader().read("XML/ShaderStageEditWidget.xml")) {
        m_xml_renderer.addDynamicBind("str_shader_file", {m_shader_file, 512, ImXML::Chars});
        m_xml_renderer.addDynamicBind("str_shader_source", {m_shader_source, 65535, ImXML::Chars});
    }

    void render() override { m_xml_renderer.render(m_xml_tree, *this); }

    void setShaderSource(const std::pair<std::string, std::string>& source, const std::filesystem::path& parent_path) {
        m_parent_path = parent_path;
        strncpy_s(m_shader_file, source.first.c_str(), 512);

        std::ifstream file(parent_path / source.first);
        std::stringstream buffer;
        buffer << file.rdbuf();

        strncpy_s(m_shader_source, buffer.str().c_str(), 65535);
    }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.type == ImXML::ImGuiEnum::TABITEM) {
            node.args["label"] = m_stage;
        } else if (node.type == ImXML::ImGuiEnum::INPUTTEXTMULTILINE) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            node.size = ImVec2(avail.x, avail.y - 50);
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_load_shader_file") {
            std::ifstream file(m_parent_path / std::string(m_shader_file));
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                strncpy_s(m_shader_source, buffer.str().c_str(), 65535);
            }
        } else if (node.arg<std::string>("id") == "btn_delete_stage") {
            ImGui::SetTabItemClosed(m_stage.c_str());
        }
    }

    std::string getShaderSource() const { return std::string(m_shader_source); }
    std::string getShaderFile() const { return std::string(m_shader_file); }

   private:
    char m_shader_file[512] = {0};
    char m_shader_source[65535] = {0};

    std::string m_stage;

    std::filesystem::path m_parent_path;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
