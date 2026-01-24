#pragma once
#include <ForwardRenderer.h>
#include <Material.h>
#include <PerspectiveCamera.h>
#include <TransformComponent.h>
#include <XMLReader.h>
#include <XMLRenderer.h>
#include <imgui.h>

#include <tuple>
#include <vector>

#include "Components/ComboBox.h"
#include "Components/InputText.h"
#include "Components/UniformInputs.h"
#include "Dialog.h"

class MaterialEditDialog : public Dialog, ImXML::XMLEventHandler {
   public:
    MaterialEditDialog() : m_xml_tree(ImXML::XMLReader().read("XML/MaterialEditPopup.xml")) {
        m_xml_renderer.addDynamicBind("bool_opaque", {&m_mtl_opaque, 1, ImXML::Bool});
        m_xml_renderer.addDynamicBind("str_material_name", {m_mtl_name, 512, ImXML::Chars});
        m_shaders_combo.onSelectionChanged([this](const std::string&, int index) { callback("shader_selected", index); });
    }

    void render() override {
        ImGui::PushID(m_dialog_id);
        if (isOpenRequested())
            ImGui::OpenPopup("Material Editor");
        m_xml_renderer.render(m_xml_tree, *this);
        ImGui::PopID();
    }

    void setPreviewTexture(void* tex) { m_preview_texture = tex; }

    void addUniformInput(const std::string& name, const ICE::UniformValue& value) {
        int ctr = m_uniform_inputs.size();
        m_uniform_inputs.emplace_back(std::make_unique<UniformInputs>(std::format("##input_{}", ctr), value));
        m_uniform_names.emplace_back(std::make_unique<InputText>(std::format("##name_{}", ctr), name));
        m_uniform_types.emplace_back(std::make_unique<ComboBox>(std::format("##type_{}", ctr), m_uniform_types_list));
        m_uniform_types.back()->setSelected(value.index());

        m_uniform_inputs.back()->onValueChanged([this, id = ctr](const auto& val) { m_material->setUniform(m_uniform_names[id]->getText(), val); });
        m_uniform_names.back()->onEdit([this](std::string prev_name, std::string new_name) { m_material->renameUniform(prev_name, new_name); });
        m_uniform_types.back()->onSelectionChanged([this, id = ctr](const std::string&, int index) {
            ICE::UniformValue val;
            switch (index) {
                case 0:
                    val = ICE::AssetUID(0);
                    break;
                case 1:
                    val = 0;
                    break;
                case 2:
                    val = 0.0f;
                    break;
                case 3:
                    val = Eigen::Vector2f();
                    break;
                case 4:
                    val = Eigen::Vector3f();
                    break;
                case 5:
                    val = Eigen::Vector4f();
                    break;
                case 6:
                    val = Eigen::Matrix4f();
                    break;
                default:
                    break;
            }
            m_uniform_inputs[id]->setValue(val);
        });

        std::vector<std::string> tex_paths;
        std::vector<ICE::AssetUID> tex_ids;
        for (const auto& [path, id] : m_textures) {
            tex_paths.push_back(path);
            tex_ids.push_back(id);
        }
        m_uniform_inputs.back()->setAssetComboList(tex_paths, tex_ids);
    }

    void setShaderList(const std::vector<std::string>& list, int selected = 0) {
        m_shaders_combo.setValues(list);
        m_shaders_combo.setSelected(selected);
    }
    void setTextureList(const std::unordered_map<std::string, ICE::AssetUID>& textures) { m_textures = textures; }
    void setMaterialName(const std::string& name) { strncpy(m_mtl_name, name.c_str(), 512); }
    void setMaterial(std::shared_ptr<ICE::Material> mtl) {
        m_material = mtl;
        m_mtl_opaque = !mtl->isTransparent();
        m_uniform_inputs.clear();
        m_uniform_names.clear();
        m_uniform_types.clear();
        for (const auto& [name, value] : mtl->getAllUniforms()) {
            addUniformInput(name, value);
        }
    }
    ICE::Material getMaterial() const { return *m_material; }
    std::string getName() const { return m_mtl_name; }

    void onNodeBegin(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "combo_shader_values") {
            m_shaders_combo.render();
        } else if (node.arg<std::string>("id") == "uniforms_table") {
            for (int i = 0; i < m_uniform_inputs.size(); i++) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                m_uniform_names[i]->render();
                ImGui::TableNextColumn();
                m_uniform_inputs[i]->render();
                ImGui::TableNextColumn();
                m_uniform_types[i]->render();
            }
        } else if (node.arg<std::string>("id") == "material_preview") {
            ImGui::Image(m_preview_texture, {256, 256}, {0, 1}, {1, 0});
        }
    }
    void onNodeEnd(ImXML::XMLNode& node) override {}
    void onEvent(ImXML::XMLNode& node) override {
        if (node.arg<std::string>("id") == "btn_apply") {
            ImGui::CloseCurrentPopup();
            done(DialogResult::Ok);
        } else if (node.arg<std::string>("id") == "btn_cancel") {
            ImGui::CloseCurrentPopup();
            done(DialogResult::Cancel);
        } else if (node.arg<std::string>("id") == "btn_add_uniform") {
            m_material->setUniform("New Uniform", 0);
            addUniformInput("New Uniform", 0);
        }
    }

   private:
    bool m_mtl_opaque;
    char m_mtl_name[512] = {0};
    void* m_preview_texture = nullptr;

    ComboBox m_shaders_combo{"###shaders", {}};

    std::vector<std::unique_ptr<UniformInputs>> m_uniform_inputs;
    std::vector<std::unique_ptr<InputText>> m_uniform_names;
    std::vector<std::unique_ptr<ComboBox>> m_uniform_types;

    std::unordered_map<std::string, ICE::AssetUID> m_textures;

    const std::vector<std::string> m_uniform_types_list = {"Texture", "int", "float", "Vector2f", "Vector3f", "Vector4f", "Matrix4f"};

    std::shared_ptr<ICE::Material> m_material;

    ImXML::XMLTree m_xml_tree;
    ImXML::XMLRenderer m_xml_renderer;
};
