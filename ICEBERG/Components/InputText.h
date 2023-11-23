#pragma once

#include <ImGUI/imgui.h>

#include <functional>
#include <string>

class InputText {
   public:
    InputText(const std::string &label, const std::string &default_text = "") : m_label(label) { setText(default_text); }
    void onEdit(const std::function<void(const std::string &)> &f) { m_callback_edit = f; }
    void render() {
        if (ImGui::InputText(m_label.c_str(), buffer, 512)) {
            m_callback_edit(std::string(buffer));
        }
        m_text = buffer;
    }
    std::string getText() const { return m_text; }
    void setText(const std::string &text) {
        m_text = text;
        memcpy(buffer, m_text.c_str(), m_text.size() + 1);
    }

   private:
    std::string m_text;
    std::string m_label;
    std::function<void(const std::string &)> m_callback_edit = [](const std::string &) {
    };

    char buffer[512] = {0};
};
