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
}
