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

        if (m_new_project_popup.getResult() == DialogResult::Ok) {
            callback("create_clicked", m_new_project_popup.getProjectDirectory(), m_new_project_popup.getProjectName());
        }
    }

    void setProjects(const std::vector<ProjectView>& projects) { m_projects = projects; }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "list_table_body") {
            renderExistingProjects();
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_create_project") {
            m_new_project_popup.open();
        } else if (node.arg<std::string>("id") == "btn_add_project") {
            auto path = open_native_dialog("*.ice");
            callback("load_clicked", path);
        }
    }

   private:
    void renderExistingProjects() {
        for (int i = 0; i < m_projects.size(); i++) {
            const auto& p = m_projects[i];
            ImGui::TableNextColumn();
            ImGui::Selectable(p.name.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
            if (ImGui::IsItemClicked()) {
                m_selected_project = i;
                callback("project_selected", i);
            }
            ImGui::TableNextColumn();
            ImGui::Text(p.modified_date.c_str());
            ImGui::TableNextColumn();
            ImGui::Text(p.path.c_str());
            ImGui::TableNextRow();

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
