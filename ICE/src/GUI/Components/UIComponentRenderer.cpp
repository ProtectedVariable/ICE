//
// Created by Thomas Ibanez on 30.11.20.
//

#include <ImGUI/imgui.h>
#include <Util/Logger.h>
#include "UIComponentRenderer.h"

namespace ICE {

    void UIComponentRenderer::render(ICE::TransformComponent *cmp) {
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
        ImGui::InputFloat("X", &vec->x());
        ImGui::SameLine();
        ImGui::InputFloat("Y", &vec->y());
        ImGui::SameLine();
        ImGui::InputFloat("Z", &vec->z());
        ImGui::PopItemWidth();
    }

    void UIComponentRenderer::render(RenderComponent *cmp, std::unordered_map<std::string, Mesh*> meshes, std::unordered_map<std::string, Material*> materials) {
        ImGui::Text("Render Component");
        //ImGui::Image();
        ImGui::Text("Meshes");
        ImGui::SameLine();
        const char* meshNames[meshes.size()];
        int i = 0;
        int selected = 0;
        for(const auto& e : meshes) {
            meshNames[i++] = e.first.c_str();
            if(e.second == cmp->getMesh()) {
                selected = i-1;
            }
        }
        ImGui::PushID("ice_mesh");
        ImGui::Combo("", &selected, meshNames, meshes.size(), 10);
        cmp->setMesh(meshes[std::string(meshNames[selected])]);
        ImGui::PopID();
        ImGui::Text("Material");
        ImGui::SameLine();
        const char* materialNames[meshes.size()];
        i = 0;
        selected = 0;
        for(const auto& e : materials) {
            materialNames[i++] = e.first.c_str();
            if(e.second == cmp->getMaterial()) {
                selected = i-1;
            }
        }
        ImGui::PushID("ice_material");
        ImGui::Combo("", &selected, materialNames, materials.size(), 10);
        ImGui::PopID();
    }
}
