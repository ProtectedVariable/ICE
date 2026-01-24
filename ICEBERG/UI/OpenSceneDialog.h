#pragma once
#include <imgui.h>

#include "Components/ComboBox.h"
#include "Components/InputText.h"
#include "Dialog.h"

class OpenSceneDialog : public Dialog {
   public:
    OpenSceneDialog(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine), m_scene_name_combo("###SceneNameCombo", {}) {
        std::vector<std::string> scenes_names;
        for (const auto& s : m_engine->getProject()->getScenes()) {
            scenes_names.push_back(s->getName());
        }
        m_scene_name_combo.setValues(scenes_names);
        m_scene_name_combo.setSelected(0);
    }

    void render() override {
        ImGui::PushID("scene_open");
        if (isOpenRequested()) {
            ImGui::OpenPopup("Scene Selection");
        }

        if (ImGui::BeginPopupModal("Scene Selection", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Select a scene to open:");
            ImGui::SameLine();
            m_scene_name_combo.render();
            if (ImGui::Button("Accept")) {
                done(DialogResult::Ok);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                done(DialogResult::Cancel);
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    int getSelectedIndex() { return m_scene_name_combo.getSelectedIndex(); }


   private:
    std::shared_ptr<ICE::ICEEngine> m_engine;
    ComboBox m_scene_name_combo;
};