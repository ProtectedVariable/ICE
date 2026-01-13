#pragma once
#include <ForwardRenderer.h>
#include <imgui.h>
#include <PerspectiveCamera.h>
#include <TransformComponent.h>

#include "Components/InputText.h"
#include "Widget.h"

class NewSceneWidget : public Widget {
   public:
    NewSceneWidget(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {}

    void render() override {
        ImGui::PushID("scene_edit");
        if (m_open) {
            ImGui::OpenPopup("Scene Editor");
            m_open = false;
        }

        if (ImGui::BeginPopupModal("Scene Editor", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            m_scene_name_in.render();
            if (ImGui::Button("Accept")) {
                m_accepted = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    std::string getSceneName() { return m_scene_name_in.getText(); }

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
    char m_name[512] = {0};
    bool m_accepted = false;
    std::shared_ptr<ICE::ICEEngine> m_engine;
    InputText m_scene_name_in{"Scene name", "New scene"};
};