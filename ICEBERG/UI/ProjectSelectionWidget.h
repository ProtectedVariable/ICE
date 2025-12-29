#pragma once
#include <XMLDynamicBind.h>
#include <XMLEventHandler.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include "NewProjectPopup.h"
#include "Widget.h"

struct ProjectView {
    std::string name;
    std::string path;
    std::string modified_date;
};

class ProjectSelectionWidget : public Widget, ImXML::XMLEventHandler {
   public:
    ProjectSelectionWidget() : m_xml_tree(ImXML::XMLReader().read("XML/ProjectSelection.xml")) {
        m_xml_renderer.addDynamicBind("str_project_search", {m_project_search, 512, ImXML::Chars});
    }

    void render() override {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        m_xml_renderer.render(m_xml_tree, *this);
        m_new_project_popup.render();
    }

    int getSelectedProject() const { return m_selected_project; }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.args.contains("id") && node.args["id"] == "list_table_body") {
            renderExistingProjects();
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.args.contains("id") && node.args["id"] == "btn_create_project") {
            m_new_project_popup.open();
        }
    }

   private:
    void renderExistingProjects() {
        ImGui::TableNextColumn();
        ImGui::Text("Load existing project");
        for (int i = 0; i < m_projects.size(); i++) {
            const auto& p = m_projects[i];
            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            ImGui::Text(p.name.c_str());
            ImGui::Text(p.path.c_str());
            ImGui::Text(p.modified_date.c_str());
            ImGui::EndGroup();
            if (ImGui::IsItemHovered()) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImVec4(0.2, 0.2, 0.2, 1)));
            }
            if (ImGui::IsItemClicked()) {
                m_selected_project = i;
                callback("project_selected", i);
            }
        }
    }

   private:
    char m_project_search[512] = {0};
    std::vector<ProjectView> m_projects;
    int m_selected_project = -1;
    NewProjectPopup m_new_project_popup;
    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
