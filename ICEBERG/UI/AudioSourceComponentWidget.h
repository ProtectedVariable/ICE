#pragma once
#include <AudioSourceComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <XMLTree.h>
#include <imgui.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

class AudioSourceComponentWidget : public Widget, ImXML::XMLEventHandler {
   public:
    explicit AudioSourceComponentWidget() : m_xml_tree(ImXML::XMLReader().read("XML/AudioSourceComponentWidget.xml")) {
        m_xml_renderer.addDynamicBind("float_volume", {&m_volume, 1, ImXML::Float});
        m_xml_renderer.addDynamicBind("float_pitch", {&m_pitch, 1, ImXML::Float});
        m_xml_renderer.addDynamicBind("bool_loop", {&m_loop, 1, ImXML::Bool});
        m_xml_renderer.addDynamicBind("bool_awake", {&m_play_on_awake, 1, ImXML::Bool});
        m_xml_renderer.addDynamicBind("bool_spatial", {&m_spatial, 1, ImXML::Bool});
        m_xml_renderer.addDynamicBind("float_min_distance", {&m_min_distance, 1, ImXML::Float});
        m_xml_renderer.addDynamicBind("float_max_distance", {&m_max_distance, 1, ImXML::Float});
        m_xml_renderer.addDynamicBind("float_rolloff", {&m_rolloff, 1, ImXML::Float});
    }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "clip_combo") {
            const std::string preview = m_selected_index >= 0 && m_selected_index < (int) m_clip_names.size()
                                            ? m_clip_names[m_selected_index]
                                            : "<none>";
            if (ImGui::BeginCombo("##audio_clip_combo", preview.c_str())) {
                for (int i = 0; i < (int) m_clip_names.size(); ++i) {
                    if (ImGui::Selectable(m_clip_names[i].c_str(), i == m_selected_index)) {
                        m_selected_index = i;
                        m_clip_changed = true;
                    }
                }
                ImGui::EndCombo();
            }
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_remove" && m_on_remove) {
            m_on_remove();
        }
    }

    void onRemove(const std::function<void()>& f) { m_on_remove = f; }
    // Fired when the editor should audition the selected clip (preview button below).
    void onPreview(const std::function<void(ICE::AssetUID)>& f) { m_on_preview = f; }

    void render() override {
        if (m_asc == nullptr) {
            return;
        }
        m_xml_renderer.render(m_xml_tree, *this);

        if (m_clip_changed && m_selected_index >= 0 && m_selected_index < (int) m_clip_ids.size()) {
            m_asc->clip = m_clip_ids[m_selected_index];
            m_clip_changed = false;
            cacheSelectedClipInfo();  // so the channel/duration readout follows the new selection
        }

        // A 3D source with a stereo clip is the single most common audio authoring mistake:
        // OpenAL positions mono buffers only, so it would play flat at full volume and look like
        // broken 3D. Say so where the mistake is made rather than only in the log at play time.
        if (m_spatial && m_selected_channels > 1) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Clip is %d-channel: cannot be spatialized.", m_selected_channels);
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Re-import it with '3D' checked to downmix to mono.");
        }
        if (m_selected_index >= 0 && m_selected_duration > 0.0f) {
            ImGui::TextDisabled("%.2fs, %d ch", m_selected_duration, m_selected_channels);
        }

        if (ImGui::Button("Preview") && m_on_preview && m_asc->clip != NO_ASSET_ID) {
            m_on_preview(m_asc->clip);
        }

        m_asc->volume = m_volume;
        m_asc->pitch = m_pitch;
        m_asc->loop = m_loop;
        m_asc->playOnAwake = m_play_on_awake;
        m_asc->spatial = m_spatial;
        m_asc->minDistance = m_min_distance;
        m_asc->maxDistance = m_max_distance;
        m_asc->rolloff = m_rolloff;
    }

    // Per-frame pointer refresh only (see TransformComponentWidget): does not rebuild the clip list
    // or re-read authored values, so typing into a field is not fought by the refresh.
    void refreshComponent(ICE::AudioSourceComponent* asc) { m_asc = asc; }

    void setAudioSourceComponent(ICE::AudioSourceComponent* asc, const std::vector<std::string>& clip_names,
                                 const std::vector<ICE::AssetUID>& clip_ids, const std::vector<int>& clip_channels,
                                 const std::vector<float>& clip_durations) {
        m_asc = asc;
        m_clip_names = clip_names;
        m_clip_ids = clip_ids;
        m_clip_channels = clip_channels;
        m_clip_durations = clip_durations;
        m_selected_index = -1;
        if (asc == nullptr) {
            return;
        }
        for (int i = 0; i < (int) m_clip_ids.size(); ++i) {
            if (m_clip_ids[i] == asc->clip) {
                m_selected_index = i;
                break;
            }
        }
        m_volume = asc->volume;
        m_pitch = asc->pitch;
        m_loop = asc->loop;
        m_play_on_awake = asc->playOnAwake;
        m_spatial = asc->spatial;
        m_min_distance = asc->minDistance;
        m_max_distance = asc->maxDistance;
        m_rolloff = asc->rolloff;
        cacheSelectedClipInfo();
    }

   private:
    void cacheSelectedClipInfo() {
        m_selected_channels = 0;
        m_selected_duration = 0.0f;
        if (m_selected_index < 0) {
            return;
        }
        if (m_selected_index < (int) m_clip_channels.size()) {
            m_selected_channels = m_clip_channels[m_selected_index];
        }
        if (m_selected_index < (int) m_clip_durations.size()) {
            m_selected_duration = m_clip_durations[m_selected_index];
        }
    }

    ICE::AudioSourceComponent* m_asc = nullptr;
    std::function<void()> m_on_remove;
    std::function<void(ICE::AssetUID)> m_on_preview;

    std::vector<std::string> m_clip_names;
    std::vector<ICE::AssetUID> m_clip_ids;
    std::vector<int> m_clip_channels;
    std::vector<float> m_clip_durations;
    int m_selected_index = -1;
    bool m_clip_changed = false;
    int m_selected_channels = 0;
    float m_selected_duration = 0.0f;

    float m_volume = 1.0f;
    float m_pitch = 1.0f;
    bool m_loop = false;
    bool m_play_on_awake = false;
    bool m_spatial = true;
    float m_min_distance = 1.0f;
    float m_max_distance = 500.0f;
    float m_rolloff = 1.0f;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
