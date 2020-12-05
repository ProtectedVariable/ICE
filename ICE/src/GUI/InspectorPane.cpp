//
// Created by Thomas Ibanez on 30.11.20.
//

#include <ImGUI/imgui.h>
#include "InspectorPane.h"

#define ICE_UID_MAX_SIZE 256
namespace ICE {

    void ICE::InspectorPane::render() {

        ImGui::Begin("Inspector");
        if(engine->getSelected() != nullptr) {
            ImGui::Text("Entity's unique ID:");
            static char buffer[ICE_UID_MAX_SIZE];
            const char* name = engine->getScene()->idByEntity(engine->getSelected()).c_str();
            strcpy(buffer, name);
            ImGui::PushID("Entity UID");
            if(ImGui::InputText("", buffer, ICE_UID_MAX_SIZE)) {
                engine->getScene()->renameEntity(name, buffer);
            }
            ImGui::Separator();
            ImGui::PopID();
            componentRenderer.render(engine->getSelected()->getComponent<TransformComponent>());
            ImGui::Separator();
            componentRenderer.render(engine->getSelected()->getComponent<RenderComponent>(), engine->getAssetBank()->getMeshes(), engine->getAssetBank()->getMaterials());
        }
        ImGui::End();
    }

    InspectorPane::InspectorPane(ICEEngine *engine): engine(engine), componentRenderer(UIComponentRenderer()) {}
}
