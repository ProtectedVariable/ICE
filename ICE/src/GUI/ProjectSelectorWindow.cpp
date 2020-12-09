//
// Created by Thomas Ibanez on 09.12.20.
//

#include "ImGUI/imgui.h"
#include <ImGUI/imgui_internal.h>
#include "ProjectSelectorWindow.h"
#include <Core/ICEEngine.h>

namespace ICE {
    ProjectSelectorWindow::ProjectSelectorWindow(ICE::ICEEngine *engine) : engine(engine) {}

    int pgui_init = 0;
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
        ImGui::SetCursorPos(ImVec2(viewport->Size.x / 4, viewport->Size.y / 2));
        if(ImGui::Button("Create New Project...")) {
            engine->setProject(new Project("", ""));
        }
        ImGui::NextColumn();
        ImGui::End();
        ImGui::PopStyleVar();
    }
}