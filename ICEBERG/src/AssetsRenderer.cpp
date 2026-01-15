#include "AssetsRenderer.h"

#include <PerspectiveCamera.h>

std::pair<void*, bool> AssetsRenderer::createThumbnail(const std::shared_ptr<ICE::Asset>& asset, const std::string& asset_path) {
    return getPreview(asset, asset_path, std::numeric_limits<float>::infinity());
}

std::pair<void*, bool> AssetsRenderer::getPreview(const std::shared_ptr<ICE::Asset>& asset, const std::string& asset_path, float t) {
    std::vector<std::shared_ptr<ICE::GPUMesh>> meshes;
    std::vector<std::shared_ptr<ICE::Material>> materials;
    std::vector<Eigen::Matrix4f> transforms;
    bool thumbnail = (t == std::numeric_limits<float>::infinity());

    auto key = thumbnail ? "thumb_" + asset_path : "preview_" + asset_path;
    if (thumbnail) {
        t = 45;
    }

    Eigen::Matrix4f rotation = ICE::rotationMatrix({0, t, 0});

    if (auto m = std::dynamic_pointer_cast<ICE::Texture2D>(asset); m) {
        return {m_bank->getTexture2D(asset_path)->ptr(), false};
    } else if (auto m = std::dynamic_pointer_cast<ICE::TextureCube>(asset); m) {
        return {nullptr, false};  //TODO
    } else if (auto m = std::dynamic_pointer_cast<ICE::Shader>(asset); m) {
        return {m_bank->getTexture2D(ICE::AssetPath::WithTypePrefix<ICE::Texture2D>("Editor/shader"))->ptr(), false};
    } else if (auto m = std::dynamic_pointer_cast<ICE::Mesh>(asset); m) {
        meshes.push_back(m_bank->getMesh(asset_path));
        materials.push_back(m_bank->getMaterial(ICE::AssetPath::WithTypePrefix<ICE::Material>("base_mat")));
        transforms.push_back(rotation);
    } else if (auto m = std::dynamic_pointer_cast<ICE::Material>(asset); m) {
        materials.push_back(m);
        meshes.push_back(m_bank->getMesh(ICE::AssetPath::WithTypePrefix<ICE::Mesh>("sphere")));
        transforms.push_back(rotation);
    } else if (auto m = std::dynamic_pointer_cast<ICE::Model>(asset); m) {
        std::vector<ICE::AssetUID> meshes_id;
        std::vector<ICE::AssetUID> materials_id;
        m->traverse(meshes_id, materials_id, transforms, rotation);
        for (int i = 0; i < meshes_id.size(); i++) {
            meshes.push_back(m_bank->getMesh(meshes_id[i]));
            materials.push_back(m_bank->getMaterial(materials_id[i]));
        }
    } else {
        return {nullptr, false};
    }


  
    if (!m_renderers.contains(key)) {
        m_renderers.try_emplace(key, m_api, m_factory);
        m_renderers.at(key).resize(256, 256);
    }

    auto camera = std::make_shared<ICE::PerspectiveCamera>(60.0, 1.0, 0.01, 10000.0);
    camera->backward(2);
    camera->up(1);
    camera->pitch(-30);

    auto& renderer = m_renderers.at(key);
    for (int i = 0; i < meshes.size(); i++) {
        std::unordered_map<ICE::AssetUID, std::shared_ptr<ICE::GPUTexture>> textures;
        for (const auto& [k, v] : materials[i]->getAllUniforms()) {
            if (std::holds_alternative<ICE::AssetUID>(v)) {
                auto id = std::get<ICE::AssetUID>(v);
                textures.try_emplace(id, m_bank->getTexture2D(id));
            }
        }
        auto shader = m_bank->getShader(materials[i]->getShader());
        if (shader)
            renderer.submitDrawable(
                ICE::Drawable{.mesh = meshes[i], .material = materials[i], .shader = shader, .textures = textures, .model_matrix = transforms[i]});
    }
    renderer.submitLight(
        ICE::Light{.position = {-2, 2, 2}, .rotation = {0, 0, 0}, .color = {1, 1, 1}, .distance_dropoff = 0, .type = ICE::LightType::PointLight});

    renderer.prepareFrame(*camera);
    auto fb = renderer.render();
    renderer.endFrame();

    return {static_cast<char*>(0) + fb->getTexture(), true};
}