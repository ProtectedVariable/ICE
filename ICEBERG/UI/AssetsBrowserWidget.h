#pragma once
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "AssetsCategoryWidget.h"
#include "AssetsContentWidget.h"

class AssetsBrowserWidget : public Widget, ImXML::XMLEventHandler {
   public:
    explicit AssetsBrowserWidget(const std::vector<std::string>& asset_categories)
        : m_xml_tree(ImXML::XMLReader().read("XML/AssetBrowser.xml")),
          m_category_widget(asset_categories) {
        m_category_widget.registerCallback("asset_category_selected", [this](int index) { callback("asset_category_selected", index); });
    }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "asset_browser_category") {
            m_category_widget.render();
        } else if (node.arg<std::string>("id") == "asset_browser_content") {
            m_content_widget.render();
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

    void render() override {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        m_xml_renderer.render(m_xml_tree, *this);
        ImGui::PopStyleVar();
    }

    void setCurrentView(const AssetView& view) { m_content_widget.setCurrentView(view); }

   private:
    AssetsCategoryWidget m_category_widget;
    AssetsContentWidget m_content_widget;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
