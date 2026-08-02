#pragma once
#include <FontAwesome/IconsFontAwesome5.h>
#include <ImGUI/ImGuizmo.h>
#include <imgui.h>

#include "Widget.h"

class ViewportWidget : public Widget {
   public:
    ViewportWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::Begin("Viewport", 0, flags);

        if (ImGui::Button(ICON_FA_ARROWS_ALT)) {
            callback("translate_clicked");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_SYNC_ALT)) {
            callback("rotate_clicked");
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_EXPAND_ALT)) {
            callback("scale_clicked");
        }

        ImVec2 pos = ImGui::GetCursorScreenPos();
        const float window_width = ImGui::GetContentRegionAvail().x;
        const float window_height = ImGui::GetContentRegionAvail().y;
        ImGui::Image(texture_ptr, {window_width, window_height}, ImVec2(0, 1), ImVec2(1, 0));
        if (ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("DND_ASSET_MODEL", target_flags)) {
                auto path = (char *) payload->Data;
                callback("spawnTree", path);
            }
            ImGui::EndDragDropTarget();
        }

        // Camera look is on the RIGHT mouse button so the left button stays free for
        // selection and gizmo manipulation. Wheel scrolls the camera forward/back.
        auto drag = ImGui::GetMouseDragDelta(1);
        if (ImGui::IsWindowHovered()) {
            if (ImGui::IsMouseDragging(1)) {
                callback("mouse_dragged", drag.x, drag.y);
                ImGui::ResetMouseDragDelta(1);
            }
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) {
                callback("scroll", wheel);
            }
            if (ImGui::IsMouseClicked(0) && !ImGuizmo::IsOver()) {
                auto m_pos = ImGui::GetMousePos();
                callback("mouse_clicked", m_pos.x - pos.x, m_pos.y - pos.y);
            }
        }

        if (ImGui::IsWindowFocused()) {
            // Frame-rate independent movement: the controller multiplies by this dt.
            float dt = ImGui::GetIO().DeltaTime;
            if (ImGui::IsKeyDown(ImGuiKey_W)) {
                callback("w_pressed", dt);
            } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
                callback("s_pressed", dt);
            }
            if (ImGui::IsKeyDown(ImGuiKey_A)) {
                callback("a_pressed", dt);
            } else if (ImGui::IsKeyDown(ImGuiKey_D)) {
                callback("d_pressed", dt);
            }
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                callback("ls_pressed", dt);
            } else if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                callback("lc_pressed", dt);
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
