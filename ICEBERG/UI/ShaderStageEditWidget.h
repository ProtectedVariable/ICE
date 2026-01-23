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
    ShaderStageEditWidget(ICE::ShaderStage stage) : m_stage(stage), m_xml_tree(ImXML::XMLReader().read("XML/ShaderStageEditWidget.xml")) {
        m_xml_renderer.addDynamicBind("str_shader_file", {m_shader_file, 512, ImXML::Chars});
        m_xml_renderer.addDynamicBind("str_shader_source", {m_shader_source, 65535, ImXML::Chars});
    }

    void render() override { m_xml_renderer.render(m_xml_tree, *this); }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.type == ImXML::ImGuiEnum::TABITEM) {
            node.args["label"] = m_stage_names.at(m_stage);
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

   private:
    char m_shader_file[512] = {0};
    char m_shader_source[65535] = {0};

    ICE::ShaderStage m_stage;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;

    const std::unordered_map<ICE::ShaderStage, std::string> m_stage_names = {
        {ICE::ShaderStage::Vertex, "Vertex"},
        {ICE::ShaderStage::Fragment, "Fragment"},
        {ICE::ShaderStage::Geometry, "Geometry"},
        {ICE::ShaderStage::TessControl, "Tessellation Control"},
        {ICE::ShaderStage::TessEval, "Tessellation Evaluation"},
        {ICE::ShaderStage::Compute, "Compute"},
    };
};
