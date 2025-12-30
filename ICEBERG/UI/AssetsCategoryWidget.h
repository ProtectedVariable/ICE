#pragma once

#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

class AssetsCategoryWidget : public Widget {
   public:
    AssetsCategoryWidget() = default;

    void render() override {
        if (ImGui::BeginTable("asset_folders", 1, ImGuiTableFlags_BordersInnerH)) {
            for (int i = 0; i < m_asset_categories.size(); i++) {
                ImGui::TableNextColumn();
                if (ImGui::Selectable(m_asset_categories[i].c_str(), i == m_selected_index)) {
                    m_selected_index = i;
                    callback("asset_category_selected", i);
                }
            }
            ImGui::EndTable();
        }
    }

   private:
    int m_selected_index = 0;
    std::vector<std::string> m_asset_categories = {"Models", "Materials", "Textures2D", "TextureCubes", "Shaders", "Others"};
};
