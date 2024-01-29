#pragma once
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>

#include "Widget.h"

class EditorWidget : public Widget {
   public:
    EditorWidget() { ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable; }

    void render() override {
        static int initialized = 0;
        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
        flags |= ImGuiWindowFlags_NoDocking;
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("ICE Editor", 0, flags);
        ImGui::PopStyleVar();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::BeginMenu("New")) {
                    if (ImGui::MenuItem("Scene")) {
                        callback("new_scene_clicked");
                    }
                    if (ImGui::MenuItem("Material")) {
                        callback("new_material_clicked");
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Import")) {
                    if (ImGui::MenuItem("Mesh")) {
                        callback("import_mesh_clicked");
                    }
                    if (ImGui::MenuItem("Texture2D")) {
                        callback("import_tex2d_clicked");
                    }
                    ImGui::EndMenu();
                }
                //if (ImGui::MenuItem("Open", "Ctrl+O")) {}
                //if (ImGui::MenuItem("Save", "Ctrl+S")) {}
                //if (ImGui::MenuItem("Save as..")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");

        if (initialized == 0) {
            initialized = 1;
            ImGui::DockBuilderRemoveNode(dockspace_id);  // Clear out existing layout
            ImGui::DockBuilderAddNode(dockspace_id);     // Add empty node

            ImGuiID dock_main_id = dockspace_id;  // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
            ImGuiID dock_top = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.80f, NULL, &dock_main_id);
            ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, NULL, &dock_main_id);

            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.20f, NULL, &dock_top);
            ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Right, 0.20f, NULL, &dock_top);
            ImGuiID dock_id_center = ImGui::DockBuilderSplitNode(dock_top, ImGuiDir_Left, 0.60f, NULL, &dock_top);

            ImGui::DockBuilderDockWindow("Assets", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Viewport", dock_id_center);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::DockSpace(dockspace_id);

        ImGui::End();
        ImGui::PopStyleVar();
    }

   private:
};
