#pragma once

#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

struct Thumbnail {
    void* ptr;
    bool flip = true;
};
struct AssetData {
    std::string name;
    Thumbnail thumbnail;
    std::string asset_path;
};
struct AssetView {
    AssetView* parent = nullptr;
    std::string folder_name;
    std::vector<AssetData> assets;
    std::vector<AssetView> subfolders;
    ICE::AssetType type;
};

class AssetsContentWidget : public Widget {
   public:
    AssetsContentWidget(void* folder_texture) : m_folder_texture(folder_texture) {}

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

            auto renderItem = [&](const std::string& label, const std::string& path, const Thumbnail& tb) {
                itemIndex++;
                ImGui::TableNextColumn();
                ImGui::PushID(itemIndex);

                if (itemIndex != m_selected_item) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                }
                ImGui::BeginGroup();
                ImVec2 uv0 = tb.flip ? ImVec2(0, 1) : ImVec2(0, 0);
                ImVec2 uv1 = tb.flip ? ImVec2(1, 0) : ImVec2(1, 1);
                ImGui::ImageButton("##item", (ImTextureID) tb.ptr, ImVec2(thumbnailSize, thumbnailSize), uv0, uv1);
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + thumbnailSize);
                ImGui::TextWrapped("%s", label.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndGroup();
                if (path != "..") {
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload(m_DND_name.c_str(), path.c_str(), path.size() + 1);
                        ImGui::Text("%s", label.c_str());
                        ImGui::EndDragDropSource();
                    }
                }

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
                renderItem("..", "..", {m_folder_texture, false});
            }
            for (const auto& folder : m_current_view.subfolders)
                renderItem(folder.folder_name, folder.folder_name, {m_folder_texture, false});

            for (const auto& asset : m_current_view.assets)
                renderItem(asset.name, asset.asset_path, asset.thumbnail);

            ImGui::EndTable();
        }
    }

    void setCurrentView(const AssetView& view) {
        m_current_view = view;
        m_selected_item = -1;
        m_current_type = view.type;
        switch (view.type) {
            case ICE::AssetType::EModel:
                m_DND_name = "DND_ASSET_MODEL";
                break;
            case ICE::AssetType::EMesh:
                m_DND_name = "DND_ASSET_MESH";
                break;
            case ICE::AssetType::EMaterial:
                m_DND_name = "DND_ASSET_MATERIAL";
                break;
            case ICE::AssetType::EShader:
                m_DND_name = "DND_ASSET_SHADER";
                break;
            case ICE::AssetType::ETex2D:
                m_DND_name = "DND_ASSET_TEXTURE2D";
                break;
            case ICE::AssetType::ETexCube:
                m_DND_name = "DND_ASSET_TEXTURECUBE";
                break;
            default:
                m_DND_name = "DND_ASSET_UNKNOWN";
                break;
        }
    }
    AssetView getCurrentView() const { return m_current_view; }

   private:
    void* m_folder_texture;
    AssetView m_current_view;
    int m_selected_item = -1;
    ICE::AssetType m_current_type = ICE::AssetType::EModel;
    std::string m_DND_name = "DND_ASSET_MODEL";
};
