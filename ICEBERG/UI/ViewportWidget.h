#pragma once
#include <ImGUI/ImGuizmo.h>
#include <ImGUI/imgui.h>

#include "Widget.h"

class ViewportWidget : public Widget {
   public:
    ViewportWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::Begin("Viewport", 0, flags);

        if (ImGui::Button("T")) {
            callback("translate_clicked");
        }
        ImGui::SameLine();
        if (ImGui::Button("R")) {
            callback("rotate_clicked");
        }
        ImGui::SameLine();
        if (ImGui::Button("S")) {
            callback("scale_clicked");
        }

        ImVec2 pos = ImGui::GetCursorScreenPos();
        const float window_width = ImGui::GetContentRegionAvail().x;
        const float window_height = ImGui::GetContentRegionAvail().y;
        ImGui::Image(texture_ptr, {window_width, window_height}, ImVec2(0, 1), ImVec2(1, 0));

        auto drag = ImGui::GetMouseDragDelta(0);
        if (ImGui::IsWindowHovered()) {
            if (ImGui::IsMouseDragging(0)) {
                callback("mouse_dragged", drag.x, drag.y);
                ImGui::ResetMouseDragDelta(0);
            } else if (ImGui::IsMouseClicked(0)) {
                auto m_pos = ImGui::GetMousePos();
                callback("mouse_clicked", m_pos.x - pos.x, m_pos.y - pos.y);
            }
        }

        if (ImGui::IsWindowFocused()) {
            if (ImGui::IsKeyDown(ImGuiKey_W)) {
                callback("w_pressed");
            } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
                callback("s_pressed");
            }
            if (ImGui::IsKeyDown(ImGuiKey_A)) {
                callback("a_pressed");
            } else if (ImGui::IsKeyDown(ImGuiKey_D)) {
                callback("d_pressed");
            }
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                callback("ls_pressed");
            } else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                callback("lc_pressed");
            }
        }
        callback("resize", window_width, window_height);
        ImGuizmo::SetRect(pos.x, pos.y, window_width, window_height);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGui::End();
    }

    void setTexture(void *texture) { texture_ptr = texture; }

   private:
    void *texture_ptr = NULL;
};
