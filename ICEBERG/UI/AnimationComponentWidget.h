#pragma once
#include <Animation.h>
#include <AnimationComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Components/UniformInputs.h"
#include "Widget.h"

class AnimationComponentWidget : public Widget, ImXML::XMLEventHandler {
   public:
    explicit AnimationComponentWidget() : m_xml_tree(ImXML::XMLReader().read("XML/AnimationComponentWidget.xml")) {
        m_xml_renderer.addDynamicBind("float_time", {&m_time, 1, ImXML::Float});
        m_xml_renderer.addDynamicBind("float_speed", {&m_speed, 1, ImXML::Float});
        m_xml_renderer.addDynamicBind("bool_playing", {&m_playing, 1, ImXML::Bool});
        m_xml_renderer.addDynamicBind("bool_loop", {&m_loop, 1, ImXML::Bool});
    }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "animation_combo") {
            if (ImGui::BeginCombo("##animation_combo", m_current_animation.c_str())) {
                for (const auto& [key, anim] : m_animations) {
                    if (ImGui::Selectable(key.c_str(), key == m_current_animation)) {
                        m_current_animation = key;
                    }
                }
                ImGui::EndCombo();
            }
        } else if (node.arg<std::string>("id") == "time_slider") {
            if (m_animations.contains(m_current_animation))
                node.args["max"] = std::to_string(m_animations[m_current_animation].duration);
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {}

    void render() override {
        if (m_ac) {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            m_xml_renderer.render(m_xml_tree, *this);
            ImGui::PopStyleVar();
            m_ac->currentAnimation = m_current_animation;
            m_ac->currentTime = m_time;
            m_ac->speed = m_speed;
            m_ac->playing = m_playing;
            m_ac->loop = m_loop;
        }
    }

    void setAnimationComponent(ICE::AnimationComponent* ac, const std::unordered_map<std::string, ICE::Animation>& animations) {
        if (ac && !animations.empty()) {
            m_ac = ac;
            m_animations = animations;
            m_current_animation = m_ac->currentAnimation;
            m_time = m_ac->currentTime;
            m_speed = m_ac->speed;
            m_playing = m_ac->playing;
            m_loop = m_ac->loop;
        } else {
            m_ac = nullptr;
        }
    }

   private:
    ICE::AnimationComponent* m_ac = nullptr;
    std::unordered_map<std::string, ICE::Animation> m_animations;

    std::string m_current_animation = "";

    float m_max_time = 1.0;

    float m_time = 0.0;
    float m_speed = 1.0;
    bool m_playing;
    bool m_loop;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
