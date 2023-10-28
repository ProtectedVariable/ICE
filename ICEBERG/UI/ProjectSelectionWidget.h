#pragma once
#include <ImGUI/imgui.h>

#include "Widget.h"

struct ProjectView {
    std::string name;
    std::string path;
    std::string modified_date;
};

class ProjectSelectionWidget : public Widget {
   public:
    ProjectSelectionWidget() = default;

    void render() override {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::Begin("ICE Project Selection", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        ImGui::BeginTable("layout_split", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable);
        ImGui::TableNextColumn();
        renderNewProjects();
        ImGui::TableNextColumn();
        renderExistingProjects();
        ImGui::EndTable();
        ImGui::End();
    }

    
    int getSelectedProject() const { return m_selected_project; }

    const char* getProjectName() const { return m_project_name; }

   private:
    void renderNewProjects() {
        ImGui::BeginTable("layout_create", 3);
        ImGui::TableNextColumn();
        ImGui::Text("Create a new Project");
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Project name: ");
        ImGui::TableNextColumn();
        ImGui::InputText("##pname", m_project_name, 512);
        ImGui::TableNextColumn();
        if (ImGui::Button("Create")) {
            callback("create_clicked");
        }
        ImGui::EndTable();
    }

    void renderExistingProjects() {
        ImGui::BeginTable("layout_projects", 1, ImGuiTableFlags_BordersH);
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
                callback("project_selected");
            }
        }
        ImGui::EndTable();
    }

   private:
    char m_project_name[512] = {0};
    std::vector<ProjectView> m_projects = {{"P0", "C:/P0", "25.10.23"}, {"P1", "C:/P1", "27.10.23"}};
    int m_selected_project = -1;
};
