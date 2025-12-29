#pragma once
#include <XMLDynamicBind.h>
#include <XMLEventHandler.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include "Widget.h"

class NewProjectPopup : public Widget, ImXML::XMLEventHandler {
   public:
    NewProjectPopup() : m_xml_tree(ImXML::XMLReader().read("XML/NewProjectPopup.xml")) {
        m_xml_renderer.addDynamicBind("str_project_name", {m_project_name, 512, ImXML::Chars});
        m_xml_renderer.addDynamicBind("str_project_directory", {m_project_dir, 512, ImXML::Chars});
    }

    void open() { m_open = true; }

    void render() override {
        if (m_open)
            ImGui::OpenPopup("New Project");
        m_open = false;
        m_xml_renderer.render(m_xml_tree, *this);
    }

    void onNodeBegin(ImXML::XMLNode& node) override {}
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

   private:
    char m_project_name[512] = {0};
    char m_project_dir[512] = {0};

    bool m_open = false;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
