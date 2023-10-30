#pragma once
#include <ImGUI/imgui.h>

#include "Widget.h"

class ViewportWidget : public Widget {
   public:
    ViewportWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::Begin("Viewport", 0, flags);

        ImGui::End();
    }

   private:
};
