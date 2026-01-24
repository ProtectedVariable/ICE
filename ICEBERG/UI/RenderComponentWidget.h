#pragma once
#include <RenderComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Components/UniformInputs.h"
#include "Widget.h"

class RenderComponentWidget : public Widget, ImXML::XMLEventHandler {
   public:
    explicit RenderComponentWidget() : m_xml_tree(ImXML::XMLReader().read("XML/RenderComponentWidget.xml")) {}

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "models_combo") {
            m_models_combo.render();
        } else if (node.arg<std::string>("id") == "materials_combo") {
            m_material_combo.render();
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

    void render() override {
        if (m_rc) {
            m_xml_renderer.render(m_xml_tree, *this);
        }
    }

    void setRenderComponent(ICE::RenderComponent* rc, const std::vector<std::string>& meshes_paths, const std::vector<ICE::AssetUID>& meshes_ids,
                            const std::vector<std::string>& materials_paths, const std::vector<ICE::AssetUID>& materials_ids) {
        m_rc = rc;
        if (m_rc) {
            m_models_combo.setValue(rc->mesh);
            m_models_combo.setAssetComboList(meshes_paths, meshes_ids);
            m_models_combo.onValueChanged([this](const ICE::UniformValue& v) { m_rc->mesh = std::get<ICE::AssetUID>(v); });

            m_material_combo.setValue(rc->material);
            m_material_combo.setAssetComboList(materials_paths, materials_ids);
            m_material_combo.onValueChanged([this](const ICE::UniformValue& v) { m_rc->material = std::get<ICE::AssetUID>(v); });
        }
    }

   private:
    ICE::RenderComponent* m_rc = nullptr;
    UniformInputs m_models_combo{"##models_combo", 0};
    UniformInputs m_material_combo{"##materials_combo", 0};

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
