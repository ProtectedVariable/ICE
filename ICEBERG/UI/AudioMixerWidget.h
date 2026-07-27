#pragma once

#include <AudioEngine.h>
#include <imgui.h>

#include <array>
#include <string>

#include "Widget.h"

// Mixer levels for the master output and each bus, plus the editor's global audio toggle.
//
// NOTE ON THE MUTE DEFAULT: this editor has no play mode -- a loaded scene is always live, so its
// AudioSystem runs and any `play on awake` source starts immediately on open. Rather than have a
// project greet you with ambient loops, editor audio starts MUTED and this panel is where you turn
// it on. If a play/stop mode is added later, that becomes the natural thing to key muting off
// instead, and this default should go away.
class AudioMixerWidget : public Widget {
   public:
    void setAudioEngine(ICE::AudioEngine* audio) { m_audio = audio; }

    void open() { m_open = true; }
    bool isOpen() const { return m_open; }

    void render() override {
        if (!m_open) {
            return;
        }
        if (ImGui::Begin("Audio Mixer", &m_open)) {
            if (m_audio == nullptr) {
                ImGui::TextDisabled("No audio service (open a project first).");
                ImGui::End();
                return;
            }

            bool muted = m_audio->isMuted();
            if (ImGui::Checkbox("Mute editor audio", &muted)) {
                m_audio->setMuted(muted);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("There is no play mode, so scenes are always live.\nEditor audio starts muted.");
            }

            ImGui::Separator();

            float master = m_audio->getMasterGain();
            if (ImGui::SliderFloat("Master", &master, 0.0f, 1.0f)) {
                m_audio->setMasterGain(master);
            }

            ImGui::Separator();
            for (std::size_t i = 0; i < kBusNames.size(); ++i) {
                const auto bus = static_cast<ICE::BusId>(i);
                ImGui::PushID(static_cast<int>(i));

                bool bus_muted = m_audio->isBusMuted(bus);
                if (ImGui::Checkbox("##mute", &bus_muted)) {
                    m_audio->setBusMuted(bus, bus_muted);
                }
                ImGui::SameLine();

                float gain = m_audio->getBusGain(bus);
                if (ImGui::SliderFloat(kBusNames[i], &gain, 0.0f, 1.0f)) {
                    m_audio->setBusGain(bus, gain);
                }
                ImGui::PopID();
            }

            ImGui::Separator();
            // A steadily climbing steal count means the voice pool is undersized for the scene --
            // worth seeing while authoring rather than discovering as sounds mysteriously dropping.
            ImGui::Text("Voices: %zu active", m_audio->getActiveVoiceCount());
            ImGui::Text("Stolen: %zu", m_audio->getStolenVoiceCount());
            ImGui::TextDisabled("Device: %s", m_audio->getBackend().deviceName().c_str());
        }
        ImGui::End();
    }

   private:
    // Parallel to the BusId enum; Master is index 0 and controlled by the master slider above, so
    // it is listed here only for completeness of the routing table.
    static inline const std::array<const char*, 5> kBusNames = {"Master bus", "Music", "SFX", "UI", "Voice"};

    ICE::AudioEngine* m_audio = nullptr;
    bool m_open = false;
};
