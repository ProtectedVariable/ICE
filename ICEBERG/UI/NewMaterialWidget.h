#pragma once
#include <ImGUI/imgui.h>
#include <TransformComponent.h>

#include "Widget.h"

class NewMaterialWidget : public Widget {
   public:
    NewMaterialWidget() = default;

    void render() override {
        if(m_open) {
            ImGui::OpenPopup("Material Editor");
            m_open = false;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if(ImGui::BeginPopupModal("Material Editor", 0, 0)) {
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void open() {
        m_open = true;
    }

   private:
    bool m_open;

};
