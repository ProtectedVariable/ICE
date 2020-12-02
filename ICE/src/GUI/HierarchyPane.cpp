//
// Created by Thomas Ibanez on 29.11.20.
//

#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>
#include <Scene/TransformComponent.h>
#include <Util/Logger.h>
#include "HierarchyPane.h"

namespace ICE {

    int ctr = 0;

    void HierarchyPane::mkPopup(const std::string& parent) {
        //Logger::Log(Logger::DEBUG, "GUI", "%s", &parent);
        ImGui::Text("Create...");
        if(ImGui::Button("3D Object")) {
            auto entity = new Entity();
            auto rc = new RenderComponent(engine->getAssetBank()->getMesh("__ice__cube__"), engine->getAssetBank()->getMaterial("__ice__base_material__"));
            auto tc = new TransformComponent();
            entity->addComponent(rc);
            entity->addComponent(tc);
            engine->getScene()->addEntity(parent, "newobject" + std::to_string(ctr++), entity);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void HierarchyPane::render() {
        selected = engine->getScene()->idByEntity(engine->getSelected());
        ImGui::Begin("Hierarchy");
        if(ImGui::IsMouseClicked(1)) {
            selected = "root";
        }
        subtree(engine->getScene()->getRoot());
        if (ImGui::BeginPopupContextWindow())
        {
            mkPopup(selected);
        }
        ImGui::End();
        if(selected != "root") {
            engine->setSelected(engine->getScene()->getByID(selected)->entity);
        }
    }

    HierarchyPane::HierarchyPane(ICEEngine* engine) : engine(engine), selected("root") {}

    void HierarchyPane::subtree(Scene::SceneNode *node) {
        for(auto c : node->children) {
            const std::string& name = engine->getScene()->idByNode(c);
            ImGuiTreeNodeFlags flags = c->children.size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0;
            if(name == selected) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            if(ImGui::TreeNodeEx(name.c_str(), flags)) {
                if(ImGui::IsItemClicked(1) || ImGui::IsItemClicked(0)) {
                    selected.assign(name);
                }
                subtree(c);
                ImGui::TreePop();
            }
        }
    }
}
