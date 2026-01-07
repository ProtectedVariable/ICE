#pragma once

#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

struct AssetData {
    std::string name;
    void* thumbnail;
    std::string asset_path;
};
struct AssetView {
    AssetView* parent = nullptr;
    std::string folder_name;
    std::vector<AssetData> assets;
    std::vector<AssetView> subfolders;
};

class AssetsContentWidget : public Widget {
   public:
    AssetsContentWidget() = default;

    void render() override {
        const float thumbnailSize = 64.0f;
        const float padding = 16.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = static_cast<int>(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        if (ImGui::BeginTable("FileBrowserTable", columnCount)) {
            int itemIndex = 0;

            auto renderItem = [&](const std::string& label, void* textureID) {
                itemIndex++;
                ImGui::TableNextColumn();
                ImGui::PushID(itemIndex);

                if (itemIndex != m_selected_item) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                }
                ImGui::BeginGroup();
                ImGui::ImageButton("##item", (ImTextureID) textureID, ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailSize);
                ImGui::TextWrapped("%s", label.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndGroup();
                if (itemIndex != m_selected_item) {
                    ImGui::PopStyleColor();
                }
                if (ImGui::IsItemClicked(0)) {
                    m_selected_item = itemIndex;
                    callback("item_selected", label);
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        callback("item_clicked", label);
                    }
                }

                ImGui::PopID();
            };

            if (m_current_view.parent != nullptr) {
                renderItem("..", nullptr);
            }
            for (const auto& folder : m_current_view.subfolders)
                renderItem(folder.folder_name, nullptr);  //TODO: Replace nullptr with folder icon

            for (const auto& asset : m_current_view.assets)
                renderItem(asset.name, asset.thumbnail);

            ImGui::EndTable();
        }
    }

    void setCurrentView(const AssetView& view) {
        m_current_view = view;
        m_selected_item = -1;
    }
    AssetView getCurrentView() const { return m_current_view; }

   private:
    AssetView m_current_view;
    int m_selected_item = -1;
};
