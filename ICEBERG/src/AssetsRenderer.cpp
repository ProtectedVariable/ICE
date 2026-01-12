#include "AssetsRenderer.h"

#include <PerspectiveCamera.h>

std::pair<void*, bool> AssetsRenderer::createThumbnail(const std::shared_ptr<ICE::Asset>& asset, const std::string& asset_path) {
    return getPreview(asset, asset_path, std::numeric_limits<float>::infinity());
}

std::pair<void*, bool> AssetsRenderer::getPreview(const std::shared_ptr<ICE::Asset>& asset, const std::string& asset_path, float t) {
    if (auto m = std::dynamic_pointer_cast<ICE::Texture2D>(asset); m) {
        return {m->getTexture(), false};
    } else if (auto m = std::dynamic_pointer_cast<ICE::TextureCube>(asset); m) {
        return {nullptr, false};  //TODO
    } else if (auto m = std::dynamic_pointer_cast<ICE::Shader>(asset); m) {
        return {m_bank->getAsset<ICE::Texture2D>(ICE::AssetPath::WithTypePrefix<ICE::Texture2D>("Editor/shader"))->getTexture(), false};
    }

    auto model = m_bank->getAsset<ICE::Model>(ICE::AssetPath::WithTypePrefix<ICE::Model>("sphere"));
    auto material = m_bank->getAsset<ICE::Material>(ICE::AssetPath::WithTypePrefix<ICE::Material>("base_mat"));
    bool override_material = false;

    if (auto m = std::dynamic_pointer_cast<ICE::Model>(asset); m) {
        model = m;
    } else if (auto m = std::dynamic_pointer_cast<ICE::Material>(asset); m) {
        material = m;
        override_material = true;
    } else {
        return {nullptr, false};
    }

    bool thumbnail = (t == std::numeric_limits<float>::infinity());

    auto key = thumbnail ? "thumb_" + asset_path : "preview_" + asset_path;
    if (thumbnail) {
        t = 45;
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
    std::vector<std::shared_ptr<ICE::Mesh>> meshes;
    std::vector<ICE::AssetUID> materials;
    std::vector<Eigen::Matrix4f> transforms;

    model->traverse(meshes, materials, transforms, ICE::rotationMatrix({0, t, 0}));
    std::unordered_map<ICE::AssetUID, std::shared_ptr<ICE::Texture>> textures;
    for (const auto& [k, v] : material->getAllUniforms()) {
        if (std::holds_alternative<ICE::AssetUID>(v)) {
            auto id = std::get<ICE::AssetUID>(v);
            textures.try_emplace(id, m_bank->getAsset<ICE::Texture2D>(id));
        }
    }
    for (int i = 0; i < meshes.size(); i++) {
        if (!override_material) {
            material = m_bank->getAsset<ICE::Material>(materials[i]);
        }
        auto shader = m_bank->getAsset<ICE::Shader>(material->getShader());
        if (shader)
            renderer.submitDrawable(
                ICE::Drawable{.mesh = meshes[i], .material = material, .shader = shader, .textures = textures, .model_matrix = transforms[i]});
    }
    renderer.submitLight(
        ICE::Light{.position = {-2, 2, 2}, .rotation = {0, 0, 0}, .color = {1, 1, 1}, .distance_dropoff = 0, .type = ICE::LightType::PointLight});

    renderer.prepareFrame(*camera);
    auto fb = renderer.render();
    renderer.endFrame();

    return {static_cast<char*>(0) + fb->getTexture(), true};
}