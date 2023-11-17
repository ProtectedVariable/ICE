#pragma once
#include <ImGUI/imgui.h>
#include <TransformComponent.h>

#include "Widget.h"

class InspectorWidget : public Widget {
   public:
    InspectorWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Inspector", 0, flags);

        if (m_tc) {
            ImGui::SeparatorText("Transform");
            ImGui::BeginGroup();

            ImGui::EndGroup();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void setTransformComponent(ICE::TransformComponent* tc) { m_tc = tc; }

   private:
    void renderVector3Input() 
    {}

    ICE::TransformComponent* m_tc = nullptr;
};
