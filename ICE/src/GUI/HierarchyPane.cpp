//
// Created by Thomas Ibanez on 29.11.20.
//

#include <ImGUI/imgui.h>
#include <Scene/TransformComponent.h>
#include <Util/Logger.h>
#include "HierarchyPane.h"
#include <Core/ICEEngine.h>
#include <Scene/LightComponent.h>

namespace ICE {

    int ctr = 0;

    void HierarchyPane::mkPopup(const std::string parent) {
        ImGui::Text("Create...");
        if(ImGui::Button("3D Object")) {
            auto entity = new Entity();
            auto rc = new RenderComponent(engine->getAssetBank()->getMesh("__ice__cube"), engine->getAssetBank()->getMaterial("__ice__base_material"));
            auto tc = new TransformComponent();
            entity->addComponent(rc);
            entity->addComponent(tc);
            engine->getScene()->addEntity(parent, "newobject" + std::to_string(ctr++), entity);
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Light Source")) {
            auto entity = new Entity();
            auto rc = new LightComponent(PointLight, Eigen::Vector3f(1,1,1));
            auto tc = new TransformComponent();
            entity->addComponent(rc);
            entity->addComponent(tc);
            engine->getScene()->addEntity(parent, "newlight" + std::to_string(ctr++), entity);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    void HierarchyPane::render() {
        selected = engine->getScene()->idByEntity(engine->getSelected());
        ImGui::Begin("Hierarchy");
        if (ImGui::BeginPopupContextWindow())
        {
            mkPopup(selected);
        } else {
            if((ImGui::IsMouseClicked(1) || ImGui::IsMouseClicked(0)) && ImGui::IsWindowHovered()) {
                selected = "root";
                engine->setSelected(nullptr);
            }
        }
        subtree(engine->getScene()->getRoot());
        ImGui::End();
        if(selected != "root") {
            engine->setSelected(engine->getScene()->getByID(selected)->entity);
        }
    }

    HierarchyPane::HierarchyPane(ICEEngine* engine) : engine(engine), selected("root") {}

    void HierarchyPane::subtree(Scene::SceneNode *node) {
        for(const auto& c : node->children) {
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
