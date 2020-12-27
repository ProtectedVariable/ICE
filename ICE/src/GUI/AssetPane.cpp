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
        ImGui::Begin("Console");
        ImGui::Text("Hello world !");
        ImGui::End();
        ImGui::Begin("Assets", nullptr, 0);
        {
            ImGui::Columns(3);
            if(init == 0) {
                ImGui::SetColumnWidth(0, 150);
                ImGui::SetColumnWidth(1, ImGui::GetWindowWidth() - 400);
                ImGui::SetColumnWidth(2, 250);
                init = 1;
            }
            ImGui::BeginChild("Asset Directories");
            ImGuiTreeNodeFlags treeflag = ImGuiTreeNodeFlags_Selected;
            if (ImGui::TreeNodeEx("Meshes", ImGuiTreeNodeFlags_Leaf | (selectedDirectory == 0 ? treeflag : 0))) {
                if (ImGui::IsItemClicked(0)) {
                    selectedDirectory = 0;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Leaf | (selectedDirectory == 1 ? treeflag : 0))) {
                if (ImGui::IsItemClicked(0)) {
                    selectedDirectory = 1;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Shaders", ImGuiTreeNodeFlags_Leaf | (selectedDirectory == 2 ? treeflag : 0))) {
                if (ImGui::IsItemClicked(0)) {
                    selectedDirectory = 2;
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Textures", ImGuiTreeNodeFlags_Leaf | (selectedDirectory == 3 ? treeflag : 0))) {
                if (ImGui::IsItemClicked(0)) {
                    selectedDirectory = 3;
                }
                ImGui::TreePop();
            }
            ImGui::EndChild();

            ImGui::NextColumn();
            contentPane.render();
            ImGui::NextColumn();
            viewPane.render();
        }
        ImGui::End();
        return true;
    }

    AssetPane::AssetPane(ICEEngine *engine) : engine(engine), viewPane(AssetViewPane(engine, &selectedAsset)), contentPane(AssetContentPane(&selectedDirectory, engine, &selectedAsset)), selectedAsset("__ice__cube") {}
}