#pragma once

#include <ImGUI/imgui.h>
#include <Material.h>

#include <functional>
#include <string>
#include <variant>

#include "ComboBox.h"

class UniformInputs {
   public:
    UniformInputs(const std::string &label, const ICE::UniformValue &value) : m_label(label), m_value(value), m_asset_combo(m_label+"_asset_combo", {}) {}
    void render() {
        std::visit([this](auto &v) { render(v); }, m_value);
    }

    void setValue(const ICE::UniformValue &value) { m_value = value; }
    ICE::UniformValue getValue() const { return m_value; }

    void onValueChanged(const std::function<void(const ICE::UniformValue &)> &f) { m_callback = f; }
    std::string getLabel() const { return m_label; }

    void setAssetComboList(const std::vector<std::string> &paths, const std::vector<ICE::AssetUID> &ids) {
        m_asset_combo.setValues(paths);
        m_assets_ids = ids;
        m_asset_combo.onSelectionChanged([this](const std::string &, int index) { m_callback(m_assets_ids[index]); });
    }

   private:
    void render(int &i) {
        if (ImGui::InputInt(m_label.c_str(), &i)) {
            m_callback(i);
        }
    }
    void render(ICE::AssetUID id) { m_asset_combo.render(); }
    void render(float &f) {
        if (ImGui::InputFloat(m_label.c_str(), &f)) {
            m_callback(f);
        }
    }
    void render(Eigen::Vector4f &v) {
        ImGui::PushID(m_label.c_str());
        ImGui::PushItemWidth(60);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        renderLabel("X", 0x990000FF);
        if (ImGui::InputFloat("##X", &v.x())) {
            m_callback(v);
        }
        ImGui::SameLine();
        renderLabel("Y", 0x9900FF00);
        if (ImGui::InputFloat("##Y", &v.y())) {
            m_callback(v);
        }
        ImGui::SameLine();
        renderLabel("Z", 0x99FF0000);
        if (ImGui::InputFloat("##Z", &v.z())) {
            m_callback(v);
        }
        ImGui::SameLine();
        renderLabel("W", 0x99FFFFFF);
        if (ImGui::InputFloat("##W", &v.w())) {
            m_callback(v);
        }
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();
        ImGui::PopID();
    }
    void render(Eigen::Matrix4f &m) {}
    void render(Eigen::Vector3f &v) {
        ImGui::PushID(m_label.c_str());
        ImGui::PushItemWidth(60);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        renderLabel("X", 0x990000FF);
        if (ImGui::InputFloat("##X", &v.x())) {
            m_callback(v);
        }
        ImGui::SameLine();
        renderLabel("Y", 0x9900FF00);
        if (ImGui::InputFloat("##Y", &v.y())) {
            m_callback(v);
        }
        ImGui::SameLine();
        renderLabel("Z", 0x99FF0000);
        if (ImGui::InputFloat("##Z", &v.z())) {
            m_callback(v);
        }
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();
        ImGui::PopID();
    }

    void renderLabel(const char *text, uint32_t backgroundColor) {
        auto dl = ImGui::GetWindowDrawList();
        ImVec2 rectSize = ImGui::CalcTextSize(text);
        rectSize.x = rectSize.y;
        ImVec2 wpos = ImGui::GetWindowPos();
        ImVec2 cpos = ImGui::GetCursorPos();
        ImVec2 pad = ImGui::GetStyle().FramePadding;
        ImVec2 cursor = ImVec2(wpos.x + cpos.x, wpos.y + cpos.y);
        ImVec2 max = ImVec2(cursor.x + rectSize.x + pad.x * 2, cursor.y + rectSize.y + pad.y * 2);
        dl->AddRectFilled(cursor, max, backgroundColor, 0.0f);
        ImGui::SetCursorPosY(cpos.y + pad.y);
        ImGui::SetCursorPosX(cpos.x + pad.x);
        ImGui::Text("%s", text);
        ImGui::SameLine(0, pad.x);
        ImGui::SetCursorPosY(cpos.y);
    }

   private:
    std::string m_label;
    ICE::UniformValue m_value;
    std::function<void(const ICE::UniformValue &)> m_callback = [](const ICE::UniformValue &) {
    };
    //Used when it's an asset input
    ComboBox m_asset_combo;
    std::vector<ICE::AssetUID> m_assets_ids;
};
