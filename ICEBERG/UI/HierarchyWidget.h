#pragma once
#include <ImGUI/imgui.h>

#include <memory>
#include <string>
#include <vector>
//#include <Entity.h>

#include "Widget.h"

enum class EntityType { LightSource, Renderable, Camera, Scene, None };

struct SceneTreeView {
    ICE::Entity id;
    std::string entity_name;
    EntityType type = EntityType::None;
    std::vector<std::shared_ptr<SceneTreeView>> children;
};

class HierarchyWidget : public Widget {
    friend class Hierarchy;

   public:
    HierarchyWidget() = default;

    void setSceneTree(const SceneTreeView& tree) { m_view = tree; }

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Hierarchy", 0, flags);

        renderTree(m_view);

        ImGui::End();
        ImGui::PopStyleVar();
    }

   private:
    void renderTree(const SceneTreeView& tree) {
        auto flags = tree.children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
        flags |= tree.id == selected_id ? ImGuiTreeNodeFlags_Selected : 0;
        flags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        auto name = tree.entity_name;
        if (ImGui::TreeNodeEx(name.c_str(), flags)) {
            if (tree.type != EntityType::Scene) {
                auto src_flags = ImGuiDragDropFlags_SourceNoDisableHover;
                if (ImGui::BeginDragDropSource(src_flags)) {
                    ImGui::SetDragDropPayload("DND_ENTITY_TREE", &tree.id, sizeof(ICE::Entity*));
                    ImGui::Text("%s", name.c_str());
                    ImGui::EndDragDropSource();
                }
            }
            if (ImGui::BeginDragDropTarget()) {
                ImGuiDragDropFlags target_flags = 0;
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY_TREE", target_flags)) {
                    auto move_from = (ICE::Entity*) payload->Data;
                    callback("hierarchy_changed", *move_from, tree.id);
                }
                ImGui::EndDragDropTarget();
            }
            if (ImGui::BeginPopupContextWindow("hierarchy_popup")) {
                if (ImGui::Button("Create new entity")) {
                    callback("create_entity_clicked", tree.id);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            if (ImGui::IsItemClicked(0)) {
                selected_id = tree.id;
                callback("selected_entity_changed", selected_id);
            }

            for (const auto& c : tree.children) {
                renderTree(*c);
            }
            ImGui::TreePop();
        }
    }

   private:
    SceneTreeView m_view;
    ICE::Entity selected_id = 0;
};
