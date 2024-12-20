#pragma once
#include <ImGUI/imgui.h>

#include "Components/ComboBox.h"
#include "Components/InputText.h"
#include "Widget.h"

class OpenSceneWidget : public Widget {
   public:
    OpenSceneWidget(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine), m_scene_name_combo("Scene", {}) {
        std::vector<std::string> scenes_names;
        for (const auto& s : m_engine->getProject()->getScenes()) {
            scenes_names.push_back(s->getName());
        }
        m_scene_name_combo.setValues(scenes_names);
        m_scene_name_combo.setSelected(0);
    }

    void render() override {
        ImGui::PushID("scene_open");
        if (m_open) {
            ImGui::OpenPopup("Scene Selection");
            m_open = false;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::BeginPopupModal("Scene Selection", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            m_scene_name_combo.render();
            if (ImGui::Button("Accept")) {
                m_accepted = true;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopID();
    }

    int getSelectedIndex() { return m_scene_name_combo.getSelectedIndex(); }

    void open() { m_open = true; }

    bool accepted() {
        if (m_accepted) {
            m_accepted = false;
            return true;
        }
        return false;
    }

   private:
    bool m_open = false;
    bool m_accepted = false;
    std::shared_ptr<ICE::ICEEngine> m_engine;
    ComboBox m_scene_name_combo;
};