//
// Created by Thomas Ibanez on 27.11.20.
//

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>
#include <Util/Logger.h>
#include <Core/ICEEngine.h>
#include "ICEGUI.h"
#include "HierarchyPane.h"

namespace ICE {
    void ICEGUI::renderImGui() {
        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
        flags |= ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Main", 0, flags);
        ImGui::PopStyleVar();

        if (ImGui::BeginMenuBar())
        {
            if(ImGui::BeginMenu("File"))
            {
                ImGui::MenuItem("New Project");
                ImGui::MenuItem("Save");
                ImGui::MenuItem("Load");
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
        ImGuiID dockspace_id = ImGui::GetID("mainspace");

        if(init == 0) {
            ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
            ImGui::DockBuilderAddNode(dockspace_id); // Add empty node

            ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
            ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
            ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.20f, NULL, &dock_main_id);
            ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.80f, NULL, &dock_main_id);

            ImGui::DockBuilderDockWindow("Assets", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
            ImGui::DockBuilderDockWindow("Viewport", dock_id_up);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderFinish(dockspace_id);
            init = 1;
        }
        ImGui::DockSpace(dockspace_id);

        ImGui::Begin("Assets");
        ImGui::End();

        hierarchyPane->render();

        ImGui::Begin("Viewport");
        ImVec2 wsize = ImGui::GetWindowSize();
        ImVec2 wpos = ImGui::GetWindowPos();
        ImGui::GetWindowDrawList()->AddImage((void *)engine->getInternalFb()->getTexture(), wpos, ImVec2(wpos.x+wsize.x, wpos.y+wsize.y), ImVec2(0, 1), ImVec2(1, 0));
        sceneViewportWidth = wsize.x;
        sceneViewportHeight = wsize.y;
        ImGui::End();

        ImGui::Begin("Inspector");
        ImGui::End();

        ImGui::End();
        ImGui::PopStyleVar();
    }

    int ICEGUI::getSceneViewportWidth() const {
        return sceneViewportWidth;
    }

    int ICEGUI::getSceneViewportHeight() const {
        return sceneViewportHeight;
    }

    ICEGUI::ICEGUI(ICEEngine *engine): engine(engine), hierarchyPane(new HierarchyPane(engine)) {

    }
}
