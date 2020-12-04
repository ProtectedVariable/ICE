//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetContentPane.h"
#include <ImGUI/imgui.h>

namespace ICE {

    void AssetContentPane::render() {
        ImGui::Begin("Asset Content");
        ImGui::Columns(6);
        if(*selectedDir == 0) {
            for(const auto& m : engine->getAssetBank()->getMeshes()) {
                //TODO: Replace with thumbnail of the mesh
                ImGui::Text("%d", m.second->getVertexArray()->getID());
                ImGui::Text("%s", m.first.c_str());
                ImGui::NextColumn();
            }
        } else if(*selectedDir == 1) {
            for(const auto& m : engine->getAssetBank()->getMaterials()) {
                //TODO: Replace with thumbnail of the material
                ImGui::Text("%s", m.first.c_str());
                ImGui::NextColumn();
            }
        } else if(*selectedDir == 2) {
            for(const auto& m : engine->getAssetBank()->getShaders()) {
                //TODO: Add file icon
                ImGui::Text("%s", m.first.c_str());
                ImGui::NextColumn();
            }
        }
        ImGui::End();
    }

    AssetContentPane::AssetContentPane(const int *selectedDir, ICEEngine *engine) : selectedDir(selectedDir),
                                                                                    engine(engine) {}
}