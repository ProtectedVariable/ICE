#pragma once
#include <ImGUI/imgui.h>

#include <memory>
#include <string>
#include <vector>
//#include <Entity.h>

#include "Widget.h"

enum class EntityType { LightSource, Renderable, Camera, Scene, None };

struct SceneTreeView {
    //ICE::Entity id; Ideally not needed, tbd
    std::string entity_name;
    EntityType type = EntityType::None;
    std::vector<std::shared_ptr<SceneTreeView>> children;
};

class HierarchyWidget : public Widget {
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
        auto name = tree.entity_name;
        if (ImGui::TreeNodeEx(name.c_str(), flags)) {
            if (tree.type != EntityType::Scene) {
                auto src_flags = ImGuiDragDropFlags_SourceNoDisableHover;
                if (ImGui::BeginDragDropSource(src_flags)) {
                    ImGui::SetDragDropPayload("DND_ENTITY_TREE", name.c_str(), name.size() + 1);
                    ImGui::Text("%s", name.c_str());
                    ImGui::EndDragDropSource();
                }
            }
            if (ImGui::BeginDragDropTarget()) {
                ImGuiDragDropFlags target_flags = 0;
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ENTITY_TREE", target_flags)) {
                    std::string move_from = (char*) payload->Data;
                    callback("hierarchy_changed");
                }
                ImGui::EndDragDropTarget();
            }

            for (const auto& c : tree.children) {
                renderTree(*c);
            }
            ImGui::TreePop();
        }
    }

   private:
    SceneTreeView m_view;
};
