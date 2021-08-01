//
// Created by Thomas Ibanez on 19.02.21.
//

#include <ImGUI/imgui.h>
#include "SceneParamPane.h"
#include <vector>
#include <Core/ICEEngine.h>

namespace ICE {

    bool SceneParamPane::render() {
        ImGui::Begin("Scene Parameters");
        ImGui::Text("Skybox");
        ImGui::SameLine();
        auto textures = std::vector<const char*>();
        static int selected = 0;
        textures.push_back("None");
        for(const auto& e : engine->getAssetBank()->getAll<Texture>()) {
            if(e.second->getType() == TextureType::CubeMap) {
                textures.push_back(engine->getAssetBank()->getName(e.first).c_str());
                if(e.first == engine->getScene()->getSkybox()->getTexture()) {
                    selected = textures.size()-1;
                }
            }
        }
        ImGui::Combo("##SkyboxCmb", &selected, textures.data(), textures.size());
        AssetUID nt = selected == 0 ? NO_ASSET_ID : engine->getAssetBank()->getUIDFromFullName(textures[selected]);
        if(engine->getScene()->getSkybox()->getTexture() != nt) {
            engine->getScene()->setSkybox(nt);
        }
        ImGui::End();
        return true;
    }

    SceneParamPane::SceneParamPane(ICEEngine *engine): engine(engine) {}

    void SceneParamPane::initialize() {

    }
}