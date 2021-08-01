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
            auto rc = new RenderComponent(engine->getAssetBank()->getUID<Mesh>("__ice__cube"), engine->getAssetBank()->getUID<Material>("__ice__base_material"), engine->getAssetBank()->getUID<Shader>("__ice__phong_shader"));
            auto tc = new TransformComponent();
            entity->addComponent(rc);
            entity->addComponent(tc);
            while(engine->getScene()->getByID("newobject" + std::to_string(++ctr)) != nullptr) {}
            engine->getScene()->addEntity(parent, "newobject" + std::to_string(ctr), entity);
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Light Source")) {
            auto entity = new Entity();
            auto rc = new LightComponent(PointLight, Eigen::Vector3f(1,1,1));
            auto tc = new TransformComponent();
            entity->addComponent(rc);
            entity->addComponent(tc);
            while(engine->getScene()->getByID("newlight" + std::to_string(++ctr)) != nullptr) {}
            engine->getScene()->addEntity(parent, "newlight" + std::to_string(ctr), entity);
            ImGui::CloseCurrentPopup();
        }
        if(selected != "root") {
            ImGui::Separator();
            if(ImGui::Button("Delete Entity")) {
                engine->getScene()->removeEntity(selected);
                selected = "root";
                engine->setSelected(nullptr);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    bool HierarchyPane::render() {
        selected = engine->getScene()->idByEntity(engine->getSelected());
        ImGui::Begin("Hierarchy");
        if(ImGui::BeginDragDropTarget()) {
            ImGuiDragDropFlags target_flags = 0;
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY_TREE", target_flags)) {
                std::string move_from = *(std::string*)payload->Data;
                engine->getScene()->setParent(move_from, "root");
            }
            ImGui::EndDragDropTarget();
        }
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
        if(engine->getScene()->getByID(selected) == nullptr) {
            selected = "root";
        }
        if(selected != "root") {
            engine->setSelected(engine->getScene()->getByID(selected)->entity);
        }
        return true;
    }

    HierarchyPane::HierarchyPane(ICEEngine* engine) : engine(engine), selected("root") {}

    void HierarchyPane::subtree(Scene::SceneNode *node) {
        for(const auto& c : node->children) {
            const std::string& name = engine->getScene()->idByNode(c);
            ImGuiTreeNodeFlags flags = c->children.size() == 0 ? ImGuiTreeNodeFlags_Leaf : 0;
            if(name == selected) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            bool expanded = ImGui::TreeNodeEx(name.c_str(), flags);
            if(ImGui::IsItemClicked(1) || ImGui::IsItemClicked(0)) {
                selected = name;
            }
            if(expanded) {
                ImGuiDragDropFlags src_flags = 0;
                src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;     // Keep the source displayed as hovered
                if(ImGui::BeginDragDropSource(src_flags)) {
                    int n = 1;
                    ImGui::SetDragDropPayload("DND_ENTITY_TREE", &name, sizeof(std::string));
                    if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                        ImGui::Text("%s", name.c_str());
                    ImGui::EndDragDropSource();
                }
                if(ImGui::BeginDragDropTarget()) {
                    ImGuiDragDropFlags target_flags = 0;
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY_TREE", target_flags)) {
                        std::string move_from = *(std::string*)payload->Data;
                        engine->getScene()->setParent(move_from, name);
                    }
                    ImGui::EndDragDropTarget();
                }
                subtree(c);
                ImGui::TreePop();
            }
        }


    }

    void HierarchyPane::initialize() { }
}
