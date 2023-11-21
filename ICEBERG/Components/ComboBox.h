#pragma once

#include <ImGUI/imgui.h>

#include <functional>
#include <string>
#include <vector>

class ComboBox {
   public:
    ComboBox(const std::string &label, const std::vector<std::string> &values) : m_values(values), m_label(label) {}

    void onSelectionChanged(const std::function<void(const std::string &, int)> &f) { m_callback_edit = f; }
    void render() {
        if (m_values.empty()) {
            return;
        }
        if (ImGui::BeginCombo(m_label.c_str(), m_values[m_selected_index].c_str())) {
            for (int i = 0; i < m_values.size(); i++) {
                bool is_selected = (m_selected_index == i);
                if (ImGui::Selectable(m_values[i].c_str(), is_selected)) {
                    m_selected_index = i;
                    m_callback_edit(m_values[m_selected_index], m_selected_index);
                }
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    std::string getSelectedItem() const { return m_values[m_selected_index]; }
    int getSelectedIndex() const { return m_selected_index; }
    void setValues(const std::vector<std::string> &values) { m_values = values; }

   private:
    std::vector<std::string> m_values;
    std::string m_label;
    std::function<void(const std::string &, int)> m_callback_edit = [](const std::string &fn, int) {
    };
    int m_selected_index = 0;
};
