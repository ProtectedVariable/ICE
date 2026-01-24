#pragma once

#include <imgui_stdlib.h>

#include <functional>
#include <string>

class InputText {
   public:
    InputText(const std::string &label, const std::string &default_text = "") : m_label(label) { 
        setText(default_text); }
    void onEdit(const std::function<void(std::string prev, std::string next)> &f) { m_callback_edit = f; }
    void render() {
        if (ImGui::InputText(m_label.c_str(), &m_text)) {
            m_callback_edit(m_text_old, m_text);
        }
        m_text_old = m_text;
    }
    std::string getText() const { return m_text; }
    void setText(const std::string &text) {
        m_text = text;
        m_text_old = text;
    }

   private:
    std::string m_text;
    std::string m_text_old;
    std::string m_label;
    std::function<void(std::string, std::string)> m_callback_edit = [](std::string, std::string) {
    };

};
