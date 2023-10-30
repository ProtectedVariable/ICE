#pragma once
#include <ImGUI/imgui.h>

#include "Widget.h"

class AssetsWidget : public Widget {
   public:
    AssetsWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Assets", 0, flags);

        ImGui::End();
        ImGui::PopStyleVar();
    }

   private:
};
