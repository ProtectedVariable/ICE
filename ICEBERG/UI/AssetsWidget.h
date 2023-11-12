#pragma once
#include <ImGUI/imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

struct AssetView {
    std::string folder_name;
    std::vector<std::pair<std::string, void*>> assets;
    std::vector<std::shared_ptr<AssetView>> subfolders;
};

class AssetsWidget : public Widget {
   public:
    AssetsWidget() = default;

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Assets", 0, flags);

        if (ImGui::BeginTable("asset_layout", 3, ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Folders", ImGuiTableColumnFlags_WidthStretch, 0.15);  
            ImGui::TableSetupColumn("Content", ImGuiTableColumnFlags_WidthStretch, 0.6);  
            ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, 0.25);       
            ImGui::TableHeadersRow();
            ImGui::TableNextColumn();
            //Left hand side: Assets folders
            if (ImGui::BeginTable("asset_folders", 1, ImGuiTableFlags_BordersInnerH)) {
                for (int i = 0; i < m_assets.size(); i++) {
                    ImGui::TableNextColumn();
                    const auto& asset = m_assets[i];
                    if (ImGui::Selectable(asset->folder_name.c_str(), i == m_selected_index)) {
                        m_selected_index = i;
                        m_current_view = m_assets[i];
                    }
                }
                ImGui::EndTable();
            }

            //Middle: assets
            ImGui::TableNextColumn();
            if (ImGui::BeginTable("assets_selection", 10)) {
                for (int i = 0; i < m_current_view->subfolders.size(); i++) {
                    const auto& folder = m_current_view->subfolders[i];
                    ImGui::TableNextColumn();
                    ImGui::Text(folder->folder_name.c_str());
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        m_current_view = folder;
                    }
                }

                for (int i = 0; i < m_current_view->assets.size(); i++) {
                    const auto& name = m_current_view->assets[i].first;
                    const auto& texture = m_current_view->assets[i].second;
                    ImGui::TableNextColumn();
                    ImGui::BeginGroup();
                    ImGui::Image(texture, {50, 50});
                    ImGui::Text(name.c_str());
                    ImGui::EndGroup();
                }
                ImGui::EndTable();
            }

            //Right hand side: preview
            ImGui::TableNextColumn();
            ImGui::Text("Preview");

            ImGui::EndTable();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void addAssets(const std::shared_ptr<AssetView> &assets) {
        m_assets.push_back(assets);
        if (m_current_view == nullptr) {
            m_current_view = assets;
        }
    }

   private:
    std::vector<std::shared_ptr<AssetView>> m_assets;
    int m_selected_index = 0;
    std::shared_ptr<AssetView> m_current_view = nullptr;
};
