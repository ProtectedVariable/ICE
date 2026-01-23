#pragma once
#include <ForwardRenderer.h>
#include <imgui.h>
#include <PerspectiveCamera.h>
#include <TransformComponent.h>

#include "Components/InputText.h"
#include "Dialog.h"

class NewSceneDialog : public Dialog {
   public:
    NewSceneDialog(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {}

    void render() override {
        ImGui::PushID("scene_edit");
        if (isOpenRequested()) {
            ImGui::OpenPopup("Scene Editor");
        }

        if (ImGui::BeginPopupModal("Scene Editor", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Scene name:");
            ImGui::SameLine();
            m_scene_name_in.render();
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

    std::string getSceneName() { return m_scene_name_in.getText(); }


   private:
    char m_name[512] = {0};
    std::shared_ptr<ICE::ICEEngine> m_engine;
    InputText m_scene_name_in{"###Scene name", "New scene"};
};