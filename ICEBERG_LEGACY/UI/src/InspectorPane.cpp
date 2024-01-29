//
// Created by Thomas Ibanez on 30.11.20.
//

#include <ImGUI/imgui.h>
#include "InspectorPane.h"
#include <ICEEngine.h>
#include <LightComponent.h>

#define ICE_UID_MAX_SIZE 256
namespace ICE {

    bool InspectorPane::render() {

        ImGui::Begin("Inspector");
        if(init < 2) { //Make sure inspector is the focused tab at the beginning
            ImGui::SetWindowFocus();
            init++;
        }
        if(engine->getSelected() != NO_ASSET_ID) {
            ImGui::Text("Entity's unique ID:");
            static char buffer[ICE_UID_MAX_SIZE];
			/*std::string oldName = engine->getScene()->idByEntity(engine->getSelected());
            strcpy(buffer, oldName.c_str());
			if(ImGui::InputText("##EntityUID", buffer, ICE_UID_MAX_SIZE)) {
                engine->getScene()->renameEntity(oldName, buffer);
            }
            ImGui::Separator();
            /*auto tc = engine->getSelected()->getComponent<TransformComponent>();
            componentRenderer.render(tc);
            ImGui::Separator();
            if(engine->getSelected()->hasComponent<RenderComponent>()) {
                auto rc = engine->getSelected()->getComponent<RenderComponent>();
                componentRenderer.render(rc);
                ImGui::Separator();
            }
            if(engine->getSelected()->hasComponent<LightComponent>()) {
                auto lc = engine->getSelected()->getComponent<LightComponent>();
                componentRenderer.render(lc);
                ImGui::Separator();
            }
            ImGui::Button("Add Component");
            if (ImGui::BeginPopupContextItem("add_cmp", ImGuiPopupFlags_MouseButtonLeft)) {
                ImGui::Text("Add Component...");
                if(!engine->getSelected()->hasComponent<RenderComponent>()) {
                    if(ImGui::Button("Render Component")) {
                        engine->getSelected()->addComponent(
                                new RenderComponent(engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Mesh>("__ice__cube")),
                                                    engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Material>("__ice__base_material")),
                                                    engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Shader>("__ice__phong_shader"))));
                        ImGui::CloseCurrentPopup();
                    }
                }
                if(!engine->getSelected()->hasComponent<LightComponent>()) {
                    if(ImGui::Button("Light Component")) {
                        engine->getSelected()->addComponent(new LightComponent(PointLight, Eigen::Vector3f(1,1,1)));
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }
            */
        }
        ImGui::End();
        return true;
    }

    InspectorPane::InspectorPane(ICEEngine *engine): engine(engine), componentRenderer(UIComponentRenderer(engine)) {}

    void InspectorPane::initialize() {  }
}
