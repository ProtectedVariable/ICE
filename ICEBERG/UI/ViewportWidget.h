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

        ImVec2 pos = ImGui::GetCursorScreenPos();
        const float window_width = ImGui::GetContentRegionAvail().x;
        const float window_height = ImGui::GetContentRegionAvail().y;

        ImGui::GetWindowDrawList()->AddImage((void *) texture_ptr, ImVec2(pos.x, pos.y), ImVec2(pos.x + window_width, pos.y + window_height), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
    }

    void setTexture(void *texture) { texture_ptr = texture; }

   private:
    void *texture_ptr = NULL;
};
