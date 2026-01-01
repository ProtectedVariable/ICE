#pragma once

#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

struct AssetView {
    AssetView *parent = nullptr;
    std::string folder_name;
    std::vector<std::pair<std::string, void *>> assets;
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
                ImGui::TableNextColumn();
                ImGui::PushID(itemIndex++);

                ImGui::BeginGroup();
                if (ImGui::ImageButton("##item", (ImTextureID) textureID, ImVec2(thumbnailSize, thumbnailSize))) {
                    // Handle Click
                }
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailSize);
                ImGui::TextWrapped("%s", label.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndGroup();

                ImGui::PopID();
            };

            for (const auto& folder : m_current_view.subfolders)
                renderItem(folder.folder_name, nullptr);  //TODO: Replace nullptr with folder icon

            for (const auto& asset : m_current_view.assets)
                renderItem(asset.first, asset.second);

            ImGui::EndTable();
        }

    }

    void setCurrentView(const AssetView &view) { m_current_view = view; }

   private:
    AssetView m_current_view;
};
