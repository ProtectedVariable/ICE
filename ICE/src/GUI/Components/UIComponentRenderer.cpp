//
// Created by Thomas Ibanez on 30.11.20.
//

#include <ImGUI/imgui.h>
#include <Util/Logger.h>
#include "UIComponentRenderer.h"
#include <Core/ICEEngine.h>

namespace ICE {

    void UIComponentRenderer::render(ICE::TransformComponent* cmp) {
        ImGui::Text("Transform Component");
        ImGui::Text("Position:");
        ImGui::PushID("ice_pos");
        renderVector3f(cmp->getPosition());
        ImGui::PopID();

        ImGui::PushID("ice_rot");
        ImGui::Text("Rotation:");
        renderVector3f(cmp->getRotation());
        ImGui::PopID();

        ImGui::PushID("ice_sca");
        ImGui::Text("Scale:");
        renderVector3f(cmp->getScale());
        ImGui::PopID();
    }

    void UIComponentRenderer::renderVector3f(Eigen::Vector3f* vec) {
        ImGui::PushItemWidth(60);
        ImGui::Text("X");
        ImGui::SameLine();
        ImGui::InputFloat("##X", &vec->x());
        ImGui::SameLine();
        ImGui::Text("Y");
        ImGui::SameLine();
        ImGui::InputFloat("##Y", &vec->y());
        ImGui::SameLine();
        ImGui::Text("Z");
        ImGui::SameLine();
        ImGui::InputFloat("##Z", &vec->z());
        ImGui::PopItemWidth();
    }

    void UIComponentRenderer::render(RenderComponent* cmp) {
        ImGui::Text("Render Component");
        ImGui::SameLine();
        ImGui::PushID("ice_rendercomponent");
        if(ImGui::Button("x")) {
            engine->getSelected()->removeComponent<RenderComponent>();
        }
        ImGui::Text("Meshes");
        ImGui::SameLine();
        auto meshNames = std::vector<const char*>(engine->getAssetBank()->getMeshes().size());
        int i = 0;
        int selected = 0;
        for(const auto& e : engine->getAssetBank()->getMeshes()) {
            meshNames[i++] = (e.first.c_str());
            if(e.second == cmp->getMesh()) {
                selected = i-1;
            }
        }
        ImGui::Combo("##Mesh", &selected, meshNames.data(), engine->getAssetBank()->getMeshes().size(), 10);
        cmp->setMesh(engine->getAssetBank()->getMesh(std::string(meshNames[selected])));
        ImGui::Text("Material");
        ImGui::SameLine();
        auto materialNames = std::vector<const char*>(engine->getAssetBank()->getMaterials().size());
        i = 0;
        selected = 0;
        for(const auto& e : engine->getAssetBank()->getMaterials()) {
            materialNames[i++] = (e.first.c_str());
            if(e.second == cmp->getMaterial()) {
                selected = i-1;
            }
        }
        ImGui::Combo("##Material", &selected, materialNames.data(), engine->getAssetBank()->getMaterials().size(), 10);
        cmp->setMaterial(engine->getAssetBank()->getMaterial(std::string(materialNames[selected])));
        ImGui::PopID();
    }

    void UIComponentRenderer::render(LightComponent* lc) {
        ImGui::Text("Light Component");
        ImGui::SameLine();
        ImGui::PushID("ice_lightcomponent");
        if(ImGui::Button("x")) {
            engine->getSelected()->removeComponent<LightComponent>();
        }
        const char *types[3] = {
                "Point Light",
                "Spot Light",
                "Directional Light"
        };
        int selected = 0;
        switch (lc->getType()) {
            case PointLight:
                selected = 0;
                break;
            case SpotLight:
                selected = 1;
                break;
            case DirectionalLight:
                selected = 2;
                break;
        }
        ImGui::Text("Light Type");
        ImGui::SameLine();
        ImGui::Combo("##Light Type", &selected, types, 3, 3);
        switch(selected) {
            case 0:
                lc->setType(PointLight);
                break;
            case 1:
                lc->setType(SpotLight);
                break;
            case 2:
                lc->setType(DirectionalLight);
                break;
        }
        ImGui::PopID();
    }

    UIComponentRenderer::UIComponentRenderer(ICEEngine *engine) : engine(engine) {}
}
