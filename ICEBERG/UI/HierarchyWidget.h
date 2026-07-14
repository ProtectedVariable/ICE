#pragma once
#include <imgui.h>

#include <cstdio>
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
        ImGui::Begin("Hierarchy", 0, flags);

        renderTree(m_view);

        // Delete key removes the selected entity (never the root scene node, id 0).
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && selected_id != 0 &&
            ImGui::IsKeyPressed(ImGuiKey_Delete)) {
            callback("delete_entity_clicked", selected_id);
        }

        renderRenamePopup();

        ImGui::End();
    }

   private:
    void renderTree(const SceneTreeView& tree) {
        // Scope the ImGui ID by entity id so two entities that share a name don't collide
        // (which corrupts selection/open state and the context popup). Paired with PopID
        // below, called unconditionally regardless of whether the node is open.
        ImGui::PushID(static_cast<int>(tree.id));
        auto flags = tree.children.empty() ? ImGuiTreeNodeFlags_Leaf : 0;
        flags |= tree.id == selected_id ? ImGuiTreeNodeFlags_Selected : 0;
        flags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
        auto name = tree.entity_name;
        if (ImGui::TreeNodeEx(name.c_str(), flags)) {
            if (tree.type != EntityType::Scene) {
                auto src_flags = ImGuiDragDropFlags_SourceNoDisableHover;
                if (ImGui::BeginDragDropSource(src_flags)) {
                    // Payload is an ICE::Entity value, not a pointer: sizeof(ICE::Entity*)
                    // would copy 4 bytes past tree.id.
                    ImGui::SetDragDropPayload("DND_ENTITY_TREE", &tree.id, sizeof(ICE::Entity));
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
            if (ImGui::BeginPopupContextItem("hierarchy_popup")) {
                if (ImGui::Button("Create new entity")) {
                    callback("create_entity_clicked", tree.id);
                    ImGui::CloseCurrentPopup();
                }
                // Duplicate/Rename/Delete only apply to real entities, not the root scene node.
                if (tree.id != 0) {
                    if (ImGui::Button("Duplicate")) {
                        callback("duplicate_entity_clicked", tree.id);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::Button("Rename")) {
                        beginRename(tree.id, tree.entity_name);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::Button("Delete")) {
                        callback("delete_entity_clicked", tree.id);
                        ImGui::CloseCurrentPopup();
                    }
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
        ImGui::PopID();
    }

    void beginRename(ICE::Entity e, const std::string& current_name) {
        m_renaming_id = e;
        std::snprintf(m_rename_buf, sizeof(m_rename_buf), "%s", current_name.c_str());
        m_open_rename = true;
        m_focus_rename = true;
    }

    void renderRenamePopup() {
        if (m_open_rename) {
            ImGui::OpenPopup("Rename Entity");
            m_open_rename = false;
        }
        if (ImGui::BeginPopup("Rename Entity")) {
            if (m_focus_rename) {
                ImGui::SetKeyboardFocusHere();
                m_focus_rename = false;
            }
            bool commit = ImGui::InputText("##rename_entity", m_rename_buf, sizeof(m_rename_buf), ImGuiInputTextFlags_EnterReturnsTrue);
            if (commit && m_rename_buf[0] != '\0') {
                callback("rename_entity", m_renaming_id, std::string(m_rename_buf));
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

   private:
    SceneTreeView m_view;
    ICE::Entity selected_id = 0;

    ICE::Entity m_renaming_id = 0;
    char m_rename_buf[256] = {0};
    bool m_open_rename = false;
    bool m_focus_rename = false;
};
