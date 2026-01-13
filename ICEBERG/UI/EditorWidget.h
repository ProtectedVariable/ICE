#pragma once
#include <XMLEventHandler.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "Widget.h"

class EditorWidget : public Widget, ImXML::XMLEventHandler {
   public:
    EditorWidget() : m_xml_tree(ImXML::XMLReader().read("XML/EditorWidget.xml")) { ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable; }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "dockspace") {
            ImGuiID dockspace_id = ImGui::GetID("MyDockspace");

            if (!m_docking_initialized) [[unlikely]] {
                m_docking_initialized = true;
                ImGui::DockBuilderRemoveNode(dockspace_id);  // Clear out existing layout
                ImGui::DockBuilderAddNode(dockspace_id);     // Add empty node

                ImGuiID dock_main_id = dockspace_id;
                ImGuiID dock_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.80f, NULL, &dock_main_id);
                ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

                ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.20f, NULL, &dock_top);
                ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Right, 0.20f, NULL, &dock_top);
                ImGuiID dock_id_center = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.60f, NULL, &dock_top);

                ImGui::DockBuilderDockWindow("Asset Browser", dock_id_bottom);
                ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
                ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
                ImGui::DockBuilderDockWindow("Viewport", dock_id_center);
                ImGui::DockBuilderFinish(dockspace_id);
            }

            ImGui::DockSpace(dockspace_id);
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

    void render() override {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        m_xml_renderer.render(m_xml_tree, *this);
    }

   private:
    bool m_docking_initialized = false;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
