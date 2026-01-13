#pragma once
#include <TransformComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Components/UniformInputs.h"
#include "Widget.h"

class TransformComponentWidget : public Widget, ImXML::XMLEventHandler {
   public:
    explicit TransformComponentWidget() : m_xml_tree(ImXML::XMLReader().read("XML/TransformComponentWidget.xml")) {
        m_transform_input.setForceVectorNumeric(true);
        m_rotation_input.setForceVectorNumeric(true);
        m_scale_input.setForceVectorNumeric(true);
    }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "position_vec") {
            m_transform_input.render();
        } else if (node.arg<std::string>("id") == "rotation_vec") {
            m_rotation_input.render();
        } else if (node.arg<std::string>("id") == "scale_vec") {
            m_scale_input.render();
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

    void render() override {
        if (m_tc) {
            m_xml_renderer.render(m_xml_tree, *this);
        }
    }

    void setTransformComponent(ICE::TransformComponent* tc) {
        m_tc = tc;
        if (tc) {
            m_transform_input.setValue(tc->getPosition());
            m_transform_input.onValueChanged([this](const ICE::UniformValue& v) { m_tc->setPosition(std::get<Eigen::Vector3f>(v)); });
            m_rotation_input.setValue(tc->getRotation());
            m_rotation_input.onValueChanged([this](const ICE::UniformValue& v) { m_tc->setRotation(std::get<Eigen::Vector3f>(v)); });
            m_scale_input.setValue(tc->getScale());
            m_scale_input.onValueChanged([this](const ICE::UniformValue& v) { m_tc->setScale(std::get<Eigen::Vector3f>(v)); });
        }
    }

   private:
    ICE::TransformComponent* m_tc = nullptr;
    UniformInputs m_transform_input{"##transform_component_position", 0};
    UniformInputs m_rotation_input{"##transform_component_rotation", 0};
    UniformInputs m_scale_input{"##transform_component_scale", 0};
    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
