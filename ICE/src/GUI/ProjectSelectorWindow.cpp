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
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        flags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
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
                engine->getConfig().getLocalProjects()->push_back(p);
                engine->setProject(&engine->getConfig().getLocalProjects()->back());
                engine->getProject()->CreateDirectories();
            }
        }
        ImGui::SetCursorPosY(viewport->Size.y);
        ImGui::NextColumn();
        ImGui::BeginChild("##Projects");
        int i = 0;
        for(auto p : *engine->getConfig().getLocalProjects()) {
            ImGui::BeginChild(("##Projects"+std::to_string(i)).c_str());
            ImGui::Text("%s", p.getName().c_str());
            ImGui::Separator();
            ImGui::EndChild();
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                engine->setProject(&(engine->getConfig().getLocalProjects()->at(i)));
            }
        }
        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleVar();
    }
}