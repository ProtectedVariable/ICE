#pragma once
#include <imgui.h>

#include "AnimationComponentWidget.h"
#include "AudioSourceComponentWidget.h"
#include "Components/InputText.h"
#include "Components/UniformInputs.h"
#include "LightComponentWidget.h"
#include "RenderComponentWidget.h"
#include "TransformComponentWidget.h"
#include "Widget.h"

class InspectorWidget : public Widget {
   public:
    InspectorWidget() {
        m_input_entity_name.onEdit([this](const std::string&, const std::string& text) { callback("entity_name_changed", text); });
        // The child component widgets own the Remove buttons; forward their clicks to this
        // widget's callback map, where the Inspector controller registers the handlers.
        m_rc_widget.onRemove([this] { callback("remove_render_component_clicked"); });
        m_lc_widget.onRemove([this] { callback("remove_light_component_clicked"); });
        m_ac_widget.onRemove([this] { callback("remove_animation_component_clicked"); });
        m_as_widget.onRemove([this] { callback("remove_audio_source_component_clicked"); });
        m_as_widget.onPreview([this](ICE::AssetUID clip) { callback("preview_audio_clip", clip); });
    }

    void render() override {
        int flags = ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoNavFocus;
        if (ImGui::Begin("Inspector", 0, flags)) {
            if (m_entity_selected) {
                ImGui::Text("Entity name");
                m_input_entity_name.render();

                m_tc_widget.render();
                m_rc_widget.render();
                m_lc_widget.render();
                m_ac_widget.render();
                m_as_widget.render();

                if (ImGui::Button("Add Component...")) {
                    callback("add_component_clicked");
                }
            }
            ImGui::End();
        }
    }

    // Refresh the widgets' cached component pointers every frame so they can't dangle
    // after another entity's structural change reallocates component storage. Unlike the
    // set* methods, this does not rebuild lists or re-bind input values.
    void refreshComponents(ICE::TransformComponent* tc, ICE::LightComponent* lc, ICE::RenderComponent* rc, ICE::AnimationComponent* ac,
                           ICE::AudioSourceComponent* as) {
        m_entity_selected = (tc != nullptr);
        m_tc_widget.refreshComponent(tc);
        m_lc_widget.refreshComponent(lc);
        m_rc_widget.refreshComponent(rc);
        m_ac_widget.refreshComponent(ac);
        m_as_widget.refreshComponent(as);
    }

    void setEntityName(const std::string& name) { m_input_entity_name.setText(name); }

    void setTransformComponent(ICE::TransformComponent* tc) {
        m_entity_selected = (tc != nullptr);
        m_tc_widget.setTransformComponent(tc);
    }
    void setLightComponent(ICE::LightComponent* lc) { m_lc_widget.setLightComponent(lc); }
    void setAnimationComponent(ICE::AnimationComponent* ac, const std::unordered_map<std::string, ICE::Animation>& animations) {
        m_ac_widget.setAnimationComponent(ac, animations);
    }
    void setRenderComponent(ICE::RenderComponent* rc, const std::vector<std::string>& meshes_paths, const std::vector<ICE::AssetUID>& meshes_ids,
                            const std::vector<std::string>& material_paths, const std::vector<ICE::AssetUID>& material_ids) {
        m_rc_widget.setRenderComponent(rc, meshes_paths, meshes_ids, material_paths, material_ids);
    }
    void setAudioSourceComponent(ICE::AudioSourceComponent* as, const std::vector<std::string>& clip_names,
                                 const std::vector<ICE::AssetUID>& clip_ids, const std::vector<int>& clip_channels,
                                 const std::vector<float>& clip_durations) {
        m_as_widget.setAudioSourceComponent(as, clip_names, clip_ids, clip_channels, clip_durations);
    }

   private:
    TransformComponentWidget m_tc_widget;
    RenderComponentWidget m_rc_widget;
    LightComponentWidget m_lc_widget;
    AnimationComponentWidget m_ac_widget;
    AudioSourceComponentWidget m_as_widget;

    bool m_entity_selected = false;

    InputText m_input_entity_name{"##inspector_entity_name", ""};
};
