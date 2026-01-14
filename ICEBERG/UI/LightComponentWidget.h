#pragma once
#include <LightComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Components/UniformInputs.h"
#include "Widget.h"

class LightComponentWidget : public Widget, ImXML::XMLEventHandler {
   public:
    explicit LightComponentWidget() : m_xml_tree(ImXML::XMLReader().read("XML/LightComponentWidget.xml")) {
        m_xml_renderer.addDynamicBind("float_distance_dropoff", {&m_distance_dropoff, 1, ImXML::Float});
    }

    void onNodeBegin(ImXML::XMLNode& node) override {
        int light_type_id = static_cast<int>(m_lc->type);
        if (node.arg<std::string>("id") == "light_type_combo") {
            node.args["preview_value"] = m_light_types[light_type_id];
        } else if (node.arg<std::string>("id") == "point_light") {
            node.args["selected"] = light_type_id == 0 ? "true" : "false";
        } else if (node.arg<std::string>("id") == "directional_light") {
            node.args["selected"] = light_type_id == 1 ? "true" : "false";
        } else if (node.arg<std::string>("id") == "spot_light") {
            node.args["selected"] = light_type_id == 2 ? "true" : "false";
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "point_light") {
            m_lc->type = ICE::PointLight;
        } else if (node.arg<std::string>("id") == "directional_light") {
            m_lc->type = ICE::DirectionalLight;
        } else if (node.arg<std::string>("id") == "spot_light") {
            m_lc->type = ICE::SpotLight;
        }
    }

    void render() override {
        if (m_lc) {
            m_xml_renderer.render(m_xml_tree, *this);
            m_lc->distance_dropoff = m_distance_dropoff;
        }
    }

    void setLightComponent(ICE::LightComponent* lc) {
        m_lc = lc;
        if (lc) {
            m_distance_dropoff = lc->distance_dropoff;
        }
    }

   private:
    ICE::LightComponent* m_lc = nullptr;

    float m_distance_dropoff;

    const std::vector<std::string> m_light_types = {"Point Light", "Directional Light", "Spot Light"};

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
