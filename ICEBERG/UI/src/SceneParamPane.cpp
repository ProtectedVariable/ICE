//
// Created by Thomas Ibanez on 19.02.21.
//

#include <ImGUI/imgui.h>
#include "SceneParamPane.h"
#include <vector>
#include <ICEEngine.h>
#include <BufferUtils.h>

namespace ICE {

    bool SceneParamPane::render() {
        ImGui::Begin("Scene Parameters");
        ImGui::Text("Skybox");
        ImGui::SameLine();
        auto textures = std::vector<AssetPath>();
        auto textureNames = std::vector<std::string>();
        static int selected = 0;
        textures.push_back(AssetPath("None"));
        textureNames.push_back("None");
        for(const auto& e : engine->getAssetBank()->getAll<TextureCube>()) {
            textures.push_back(engine->getAssetBank()->getName(e.first));
            textureNames.push_back(engine->getAssetBank()->getName(e.first).getName());
            if(e.first == engine->getScene()->getSkybox()->getTexture()) {
                selected = textures.size()-1;
            }
        }
        ImGui::Combo("##SkyboxCmb", &selected, BufferUtils::CreateCharBuffer(textureNames).data(), textures.size());
        AssetUID nt = selected == 0 ? NO_ASSET_ID : engine->getAssetBank()->getUID(textures[selected]);
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