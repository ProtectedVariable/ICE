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
            renderVector3Input(m_tc->position, "position");
            renderVector3Input(m_tc->rotation, "rotation");
            renderVector3Input(m_tc->scale, "scale");
            ImGui::EndGroup();
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void setTransformComponent(ICE::TransformComponent* tc) { m_tc = tc; }

   private:
    void renderVector3Input(Eigen::Vector3f &input, const char* id) {
        ImGui::PushID(id);
        ImGui::PushItemWidth(60);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        renderLabel("X", 0x990000FF);
        ImGui::InputFloat("##X", &input.x());
        ImGui::SameLine();
        renderLabel("Y", 0x9900FF00);
        ImGui::InputFloat("##Y", &input.y());
        ImGui::SameLine();
        renderLabel("Z", 0x99FF0000);
        ImGui::InputFloat("##Z", &input.z());
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();
        ImGui::PopID();
    }

     void renderLabel(const char *text, uint32_t backgroundColor) {
        auto dl = ImGui::GetWindowDrawList();
        ImVec2 rectSize = ImGui::CalcTextSize(text);
        rectSize.x = rectSize.y;
        ImVec2 wpos = ImGui::GetWindowPos();
        ImVec2 cpos = ImGui::GetCursorPos();
        ImVec2 pad = ImGui::GetStyle().FramePadding;
        ImVec2 cursor = ImVec2(wpos.x + cpos.x, wpos.y + cpos.y);
        ImVec2 max = ImVec2(cursor.x + rectSize.x + pad.x * 2, cursor.y + rectSize.y + pad.y * 2);
        dl->AddRectFilled(cursor, max, backgroundColor, 0.0f);
        ImGui::SetCursorPosY(cpos.y + pad.y);
        ImGui::SetCursorPosX(cpos.x + pad.x);
        ImGui::Text("%s", text);
        ImGui::SameLine(0, pad.x);
        ImGui::SetCursorPosY(cpos.y);
    }

    ICE::TransformComponent* m_tc = nullptr;
};
