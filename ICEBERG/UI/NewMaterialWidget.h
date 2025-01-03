#pragma once
#include <ForwardRenderer.h>
#include <ImGUI/imgui.h>
#include <Material.h>
#include <PerspectiveCamera.h>
#include <TransformComponent.h>

#include <tuple>
#include <vector>

#include "Components/ComboBox.h"
#include "Components/InputText.h"
#include "Components/UniformInputs.h"
#include "Widget.h"

class NewMaterialWidget : public Widget {
   public:
    NewMaterialWidget(const std::shared_ptr<ICE::ICEEngine>& engine) : m_engine(engine) {}

    void render() override {
        ImGui::PushID(m_id);
        if (m_open) {
            ImGui::OpenPopup("Material Editor");
            m_open = false;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::BeginPopupModal("Material Editor", 0, 0)) {
            if (ImGui::BeginTable("mat_editor_layout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
                ImGui::TableNextColumn();
                ImGui::Text("Name:");
                ImGui::SameLine();
                ImGui::InputText("##name", m_name, 512);
                ImGui::Text("Shader:");
                ImGui::SameLine();
                m_shaders_combo.render();
                ImGui::Text("Uniforms");
                ImGui::SameLine();
                if (ImGui::BeginTable("uniforms_table", 3)) {
                    ImGui::TableSetupColumn("Name");
                    ImGui::TableSetupColumn("Value");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableHeadersRow();
                    for (int i = 0; i < m_uniform_names.size(); i++) {
                        ImGui::TableNextColumn();
                        m_uniform_names[i].render();
                        ImGui::TableNextColumn();
                        m_uniform_inputs[i].render();
                        ImGui::TableNextColumn();
                        m_uniform_combos[i].render();
                    }
                    ImGui::EndTable();
                }

                if (ImGui::Button("New Uniform")) {
                    addUniformInput("uNew", 0);
                }

                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    m_id = 0;
                }
                ImGui::SameLine();
                if (ImGui::Button("Apply")) {
                    auto new_name = m_engine->getAssetBank()->getName(m_id).prefix() + m_name;
                    auto rename_ok = m_engine->getAssetBank()->renameAsset(m_engine->getAssetBank()->getName(m_id),
                                                                           new_name);
                    if (rename_ok) {
                        m_accepted = true;
                        ImGui::CloseCurrentPopup();
                        m_id = 0;
                    }
                }
                ImGui::TableNextColumn();
                ImGui::Image(renderPreview(), {256, 256});
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
        ImGui::PopID();
    }

    void open(ICE::AssetUID id) {
        m_id = id;
        m_open = true;
        m_material = m_engine->getAssetBank()->getAsset<ICE::Material>(id);
        auto shaders = m_engine->getAssetBank()->getAll<ICE::Shader>();
        std::vector<std::string> shader_names;
        int shader_idx = 0;
        int i = 0;
        for (const auto& [id, shader] : shaders) {
            shader_names.push_back(m_engine->getAssetBank()->getName(id).toString());
            if (id == m_material->getShader()) {
                shader_idx = i;
            }
            i++;
        }
        m_shaders_combo.setValues(shader_names);
        m_shaders_combo.setSelected(shader_idx);
        auto name = m_engine->getAssetBank()->getName(id).getName();
        memcpy(m_name, name.c_str(), name.size() + 1);

        m_shaders_combo.onSelectionChanged([this](const std::string& name, int) { m_material->setShader(m_engine->getAssetBank()->getUID(name)); });

        m_uniform_names.clear();
        m_uniform_combos.clear();
        m_uniform_inputs.clear();
        for (const auto& [uname, uniform] : m_material->getAllUniforms()) {
            addUniformInput(uname, uniform);
        }
    }

    void addUniformInput(const std::string& uname, const ICE::UniformValue& value) {
        m_uniform_names.emplace_back("##uName_" + std::to_string(m_ctr), uname);
        m_uniform_combos.emplace_back("##uType_" + std::to_string(m_ctr), uniform_types_names);
        m_uniform_inputs.emplace_back("##uIn_" + std::to_string(m_ctr), value);

        m_uniform_inputs.back().onValueChanged(
            [this, in = m_uniform_inputs.size() - 1](const ICE::UniformValue& v) { m_material->setUniform(m_uniform_names[in].getText(), v); });

        auto textures = m_engine->getAssetBank()->getAll<ICE::Texture2D>();
        std::vector<ICE::AssetUID> uids;
        std::vector<std::string> paths;
        for (const auto& [id, ptr] : textures) {
            uids.push_back(id);
            paths.push_back(m_engine->getAssetBank()->getName(id).toString());
        }
        if (std::holds_alternative<ICE::AssetUID>(value)) {
            m_uniform_inputs.back().setAssetComboList(paths, uids);
        }

        m_uniform_combos.back().setSelected(comboIDFromValue(value));
        m_uniform_combos.back().onSelectionChanged([this, in = m_uniform_inputs.size() - 1](const std::string& selected, int i) {
            if (i == 0) {
                m_uniform_inputs[in].setValue(ICE::AssetUID(0));
            } else if (i == 1) {
                m_uniform_inputs[in].setValue(0);
            } else if (i == 2) {
                m_uniform_inputs[in].setValue(0.0f);
            } else if (i == 3) {
                m_uniform_inputs[in].setValue(Eigen::Vector3f(0, 0, 0));
            } else if (i == 4) {
                m_uniform_inputs[in].setValue(Eigen::Vector4f(0, 0, 0, 0));
            } else if (i == 4) {
                m_uniform_inputs[in].setValue(Eigen::Matrix4f());
            }
        });

        m_ctr++;
    }

    int comboIDFromValue(const ICE::UniformValue& value) {
        if (std::holds_alternative<float>(value)) {
            return 2;
        } else if (!std::holds_alternative<ICE::AssetUID>(value) && std::holds_alternative<int>(value)) {
            return 1;
        } else if (std::holds_alternative<ICE::AssetUID>(value)) {
            return 0;
        } else if (std::holds_alternative<Eigen::Vector3f>(value)) {
            return 3;
        } else if (std::holds_alternative<Eigen::Vector4f>(value)) {
            return 4;
        } else if (std::holds_alternative<Eigen::Matrix4f>(value)) {
            return 5;
        } else {
            throw std::runtime_error("Uniform type not implemented");
        }
    }

    void* renderPreview() {
        auto model_uid = m_engine->getAssetBank()->getUID(ICE::AssetPath::WithTypePrefix<ICE::Model>("sphere"));

        auto model = m_engine->getAssetBank()->getAsset<ICE::Model>(model_uid);
        auto shader = m_engine->getAssetBank()->getAsset<ICE::Shader>(m_material->getShader());

        auto camera = std::make_shared<ICE::PerspectiveCamera>(60.0, 1.0, 0.01, 10000.0);
        camera->backward(2);
        camera->up(1);
        camera->pitch(-30);

        shader->bind();
        shader->loadMat4("projection", camera->getProjection());
        shader->loadMat4("view", camera->lookThrough());

        ICE::GeometryPass pass(m_engine->getApi(), m_engine->getGraphicsFactory(), {256, 256, 1});
        std::vector<ICE::RenderCommand> cmds;
        for (const auto& mesh : model->getMeshes()) {
            cmds.push_back(ICE::RenderCommand{.mesh = mesh, .material = m_material, .shader = shader, .model_matrix = Eigen::Matrix4f::Identity()});
        }
        pass.submit(&cmds);
        pass.execute();

        return static_cast<char*>(0) + pass.getResult()->getTexture();
    }

    bool accepted() {
        if (m_accepted) {
            m_accepted = false;
            return true;
        }
        return false;
    }

   private:
    bool m_open = false;
    char m_name[512] = {0};
    int m_shader_index = 0;
    ComboBox m_shaders_combo{"##shader_combo", {}};
    std::vector<InputText> m_uniform_names;
    std::vector<ComboBox> m_uniform_combos;
    std::vector<UniformInputs> m_uniform_inputs;
    const std::vector<std::string> uniform_types_names = {"Asset", "Int", "Float", "Vector3", "Vector4", "Matrix4"};
    int m_ctr = 0;
    bool m_accepted = false;
    std::shared_ptr<ICE::ICEEngine> m_engine;
    std::shared_ptr<ICE::Material> m_material;
    ICE::AssetUID m_id;
};
