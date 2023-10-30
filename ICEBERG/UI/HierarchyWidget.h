#pragma once
#include <ImGUI/imgui.h>

#include "Widget.h"

class HierarchyWidget : public Widget {
   public:
    HierarchyWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Hierarchy", 0, flags);
        
        ImGui::End();
        ImGui::PopStyleVar();
    }

   private:
};
