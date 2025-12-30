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
    AssetsBrowserWidget() : m_xml_tree(ImXML::XMLReader().read("XML/AssetBrowser.xml")) {}

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "asset_browser_category") {
            m_category_widget.render();
        } else if (node.arg<std::string>("id") == "asset_browser_content") {
            m_content_widget.render();
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

    void render() override { m_xml_renderer.render(m_xml_tree, *this); }

   private:
    AssetsCategoryWidget m_category_widget;
    AssetsContentWidget m_content_widget;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
