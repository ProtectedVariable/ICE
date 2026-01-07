#pragma once

#include <AssetBank.h>
#include <ForwardRenderer.h>

#include <unordered_map>

class AssetsRenderer {
   public:
    AssetsRenderer(const std::shared_ptr<ICE::RendererAPI>& api, const std::shared_ptr<ICE::GraphicsFactory>& factory,
                   const std::shared_ptr<ICE::AssetBank>& bank)
        : m_api(api),
          m_factory(factory),
          m_bank(bank) {}

    void* createThumbnail(const std::shared_ptr<ICE::Asset>& asset, const std::string& path);
    void* getPreview(const std::shared_ptr<ICE::Asset>& asset, const std::string &path, float t);

   private:
    std::unordered_map<std::string, ICE::ForwardRenderer> m_renderers;
    std::shared_ptr<ICE::RendererAPI> m_api;
    std::shared_ptr<ICE::GraphicsFactory> m_factory;
    std::shared_ptr<ICE::AssetBank> m_bank;
};
