#pragma once

#include <ImGUI/imgui.h>

#include <functional>
#include <string>

class InputText {
   public:
    InputText(const std::string &label, const std::string &default_text = "") : m_text(default_text), m_label(label) {
        memcpy(buffer, default_text.c_str(), default_text.size() + 1);
    }
    void onEdit(const std::function<void(const std::string &)> &f) { m_callback_edit = f; }
    void render() {
        ImGui::InputText(m_label.c_str(), buffer, 512);
        m_text = buffer;
    }
    std::string getText() { return m_text; }

   private:
    std::string m_text;
    std::string m_label;
    std::function<void(const std::string &)> m_callback_edit = [](const std::string &) {
    };

    char buffer[512] = {0};
};
