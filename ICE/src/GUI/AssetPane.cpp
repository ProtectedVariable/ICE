//
// Created by Thomas Ibanez on 03.12.20.
//

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>
#include "AssetPane.h"
#include "AssetViewPane.h"
#include "AssetContentPane.h"
#include <Core/ICEEngine.h>

namespace ICE {

    int init = 0;
    bool AssetPane::render() {
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar;
        ImGui::Begin("Assets", nullptr, flags);
        ImGuiID dockspace_id = ImGui::GetID("assetspace");
        if(init == 0) {
            ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout
            ImGui::DockBuilderAddNode(dockspace_id); // Add empty node

            ImGuiID dock_main_id = dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.
            ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, NULL, &dock_main_id);
            ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.30f, NULL, &dock_main_id);
            ImGuiID dock_id_up = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Up, 0.80f, NULL, &dock_main_id);

            ImGui::DockBuilderDockWindow("Asset Directories", dock_id_left);
            ImGui::DockBuilderDockWindow("Asset Content", dock_id_up);
            ImGui::DockBuilderDockWindow("Console", dock_id_up);
            ImGui::DockBuilderDockWindow("Asset View", dock_id_right);
            ImGui::DockBuilderFinish(dockspace_id);
            init = 1;
        }
        ImGui::DockSpace(dockspace_id);
        ImGui::End();

        ImGui::Begin("Asset Directories");
        ImGuiTreeNodeFlags treeflag = ImGuiTreeNodeFlags_Selected;
        if(ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_Leaf | (selectedDirectory == 0 ? treeflag : 0))) {
            if(ImGui::IsItemClicked(0)) {
                selectedDirectory = 0;
            }
            ImGui::TreePop();
        }
        if(ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Leaf | (selectedDirectory == 1 ? treeflag : 0))) {
            if(ImGui::IsItemClicked(0)) {
                selectedDirectory = 1;
            }
            ImGui::TreePop();
        }
        if(ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_Leaf | (selectedDirectory == 2 ? treeflag : 0))) {
            if(ImGui::IsItemClicked(0)) {
                selectedDirectory = 2;
            }
            ImGui::TreePop();
        }
        ImGui::End();

        ImGui::Begin("Console");
        ImGui::Text("Hello world !");
        ImGui::End();

        contentPane.render();
        viewPane.render();
        return true;
    }

    AssetPane::AssetPane(ICEEngine *engine) : engine(engine), viewPane(AssetViewPane(engine, &selectedAsset)), contentPane(AssetContentPane(&selectedDirectory, engine, &selectedAsset)), selectedAsset(engine->getAssetBank()->getMeshes().begin()->first) {}
}