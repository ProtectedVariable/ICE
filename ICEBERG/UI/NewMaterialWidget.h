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
                    m_uniform_names.emplace_back("##uName_" + std::to_string(m_ctr), "uNew");
                    m_uniform_combos.emplace_back("##uType_" + std::to_string(m_ctr), uniform_types_names);
                    m_uniform_inputs.emplace_back("##uIn_" + std::to_string(m_ctr), 0);

                    m_uniform_combos.back().onSelectionChanged([this, in = m_uniform_inputs.size() - 1](const std::string& selected, int i) {
                        switch (i) {
                            case 0:
                                m_uniform_inputs[in].setValue(ICE::AssetUID(0));
                                break;
                            case 1:
                                m_uniform_inputs[in].setValue(0);
                                break;
                            case 2:
                                m_uniform_inputs[in].setValue(.0f);
                                break;
                            case 3:
                                m_uniform_inputs[in].setValue(Eigen::Vector3f(0, 0, 0));
                                break;
                            case 4:
                                m_uniform_inputs[in].setValue(Eigen::Vector4f(0, 0, 0, 0));
                                break;
                            case 5:
                                m_uniform_inputs[in].setValue(Eigen::Matrix4f());
                                break;
                            default:
                                throw std::invalid_argument("Out of bounds selection");
                        }
                    });

                    m_uniform_inputs.back().onValueChanged([this, in = m_uniform_inputs.size() - 1](const ICE::UniformValue& v) {
                        m_material->setUniform(m_uniform_names[in].getText(), v);
                    });

                    m_ctr++;
                }

                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Apply")) {
                    auto material = std::make_shared<ICE::Material>();
                    material->setShader(m_engine->getAssetBank()->getUID(m_shaders_combo.getSelectedItem()));
                    for (int i = 0; i < m_uniform_names.size(); i++) {
                        material->setUniform(m_uniform_names[i].getText(), m_uniform_inputs[i].getValue());
                    }
                    m_engine->getAssetBank()->renameAsset(m_engine->getAssetBank()->getName(m_id),
                                                          ICE::AssetPath::WithTypePrefix<ICE::Material>(m_name));
                    ImGui::CloseCurrentPopup();
                }
                ImGui::TableNextColumn();
                ImGui::Image(renderPreview(), {256, 256});
                ImGui::EndTable();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void open(ICE::AssetUID id) {
        m_id = id;
        m_open = true;
        auto shaders = m_engine->getAssetBank()->getAll<ICE::Shader>();
        std::vector<std::string> shader_names;
        for (const auto& [id, shader] : shaders) {
            shader_names.push_back(m_engine->getAssetBank()->getName(id).toString());
        }
        m_shaders_combo.setValues(shader_names);
        auto name = m_engine->getAssetBank()->getName(id).toString();
        memcpy(m_name, name.c_str(), name.size() + 1);

        m_material = m_engine->getAssetBank()->getAsset<ICE::Material>(id);
        m_material->setShader(shaders.begin()->first);

        m_shaders_combo.onSelectionChanged([this](const std::string& name, int) { m_material->setShader(m_engine->getAssetBank()->getUID(name)); });
    }

    /*
    void setUniforms(const std::vector<std::tuple<std::string, ICE::UniformValue, std::string>>& uniforms) {
        for (const auto& u : uniforms) {
            const auto& name = std::get<0>(u);
            const auto& val = std::get<1>(u);
            const auto& type = std::get<2>(u);
            m_uniform_names.emplace_back("##uName_" + std::to_string(m_ctr), name);
            m_uniform_combos.emplace_back("##uType_" + std::to_string(m_ctr), uniform_types_names);
            m_uniform_inputs.emplace_back("##uIn_" + std::to_string(m_ctr), val);
            m_ctr++;
        }
    }
    */

    void* renderPreview() {
        auto preview_framebuffer = m_engine->getGraphicsFactory()->createFramebuffer({256, 256, 1});
        ICE::Scene s("preview_scene");

        auto render_system = std::make_shared<ICE::RenderSystem>();
        render_system->setRenderer(std::make_shared<ICE::ForwardRenderer>(m_engine->getApi(), s.getRegistry(), m_engine->getAssetBank()));

        auto camera = std::make_shared<ICE::PerspectiveCamera>(60.0, 1.0, 0.01, 10000.0);
        camera->backward(2);
        camera->up(1);
        camera->pitch(-30);
        render_system->setCamera(camera);

        s.getRegistry()->addSystem(render_system);

        auto entity = s.createEntity();
        auto mesh_uid = m_engine->getAssetBank()->getUID(ICE::AssetPath("Meshes/sphere"));

        s.getRegistry()->addComponent<ICE::RenderComponent>(entity, ICE::RenderComponent(mesh_uid, m_id));
        s.getRegistry()->addComponent<ICE::TransformComponent>(entity, ICE::TransformComponent({0, 0, 0}, {0, 45, 0}, {1, 1, 1}));
        render_system->setTarget(preview_framebuffer);
        render_system->update(1.0f);

        return preview_framebuffer->getTexture();
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
    std::shared_ptr<ICE::ICEEngine> m_engine;
    std::shared_ptr<ICE::Material> m_material;
    ICE::AssetUID m_id;
};
