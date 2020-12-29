//
// Created by Thomas Ibanez on 09.12.20.
//

#include "ImGUI/imgui.h"
#include <ImGUI/imgui_internal.h>
#include "ProjectSelectorWindow.h"
#include <Core/ICEEngine.h>
#include <Platform/FileUtils.h>

namespace ICE {
    ProjectSelectorWindow::ProjectSelectorWindow(ICE::ICEEngine *engine) : engine(engine) {}

    int pgui_init = 0;
    char project_name_buffer[64] = {0};
    void ProjectSelectorWindow::render() {
        ImGuiWindowFlags flags = 0;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Project Selection", 0, flags);
        ImGui::PopStyleVar();
        ImGui::Columns(2);
        ImGui::SetCursorPosY(viewport->Size.y / 2);
        ImGui::Text("Project Name");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(300);
        ImGui::InputText("##New Project Name", project_name_buffer, 64);
        ImGui::SameLine();
        if(ImGui::Button("Create New Project...") && strcmp(project_name_buffer, "") != 0) {
            std::string baseFolder = FileUtils::openFolderDialog();
            if(baseFolder != "") {
                Project p = Project(baseFolder, project_name_buffer);
                p.CreateDirectories();
                engine->getConfig().getLocalProjects()->push_back(p);
                engine->setProject(&engine->getConfig().getLocalProjects()->back());
            }
        }
        ImGui::SetCursorPosY(viewport->Size.y);
        ImGui::NextColumn();
        int i = 0;
        static std::string hovered;
        static ImVec2 endCursor;
        for(auto p : *engine->getConfig().getLocalProjects()) {
            if(hovered == p.getName()) {
                ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, ImGui::GetCursorScreenPos().y), ImVec2(endCursor.x * ImGui::GetWindowDpiScale(), endCursor.y), 0x88888888);
            }
            ImGui::BeginGroup();
            ImGui::Text("Name: %s", p.getName().c_str());
            ImGui::Text("%s", p.getBaseDirectory().c_str());
            ImGui::SetCursorPosX(ImGui::GetWindowWidth());
            ImGui::EndGroup();
            if (ImGui::IsItemHovered()) {
                hovered = p.getName();
                endCursor = ImGui::GetCursorScreenPos();
                endCursor.x = ImGui::GetWindowWidth();
                if(ImGui::IsMouseClicked(0)) {
                    engine->setProject(&(engine->getConfig().getLocalProjects()->at(i)));
                }
            }
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
            i++;
        }
        if(ImGui::GetMousePos().y > ImGui::GetCursorScreenPos().y || ImGui::GetMousePos().x < ImGui::GetCursorScreenPos().x) {
            hovered = "";
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
}