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

class MaterialEditDialog : public Dialog, ImXML::XMLEventHandler {
   public:
    MaterialEditDialog() : m_xml_tree(ImXML::XMLReader().read("XML/MaterialEditPopup.xml")) {
        m_xml_renderer.addDynamicBind("bool_opaque", {&m_mtl_opaque, 1, ImXML::Bool});
        m_xml_renderer.addDynamicBind("str_material_name", {m_mtl_name, 512, ImXML::Chars});
    }

    void render() override {
        if (isOpenRequested())
            ImGui::OpenPopup("Material Editor");
        m_xml_renderer.render(m_xml_tree, *this);
    }

    void onNodeBegin(ImXML::XMLNode& node) override {}
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
    bool m_mtl_opaque;
    char m_mtl_name[512];

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
