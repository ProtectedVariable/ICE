#pragma once

#include <GPURegistry.h>
#include <ForwardRenderer.h>

#include <unordered_map>

class AssetsRenderer {
   public:
    AssetsRenderer(const std::shared_ptr<ICE::RendererAPI>& api, const std::shared_ptr<ICE::GraphicsFactory>& factory,
                   const std::shared_ptr<ICE::GPURegistry>& bank)
        : m_api(api),
          m_factory(factory),
          m_bank(bank) {}

    std::pair<void*, bool> createThumbnail(const std::shared_ptr<ICE::Asset>& asset, const std::string& path);
    std::pair<void*, bool> getPreview(const std::shared_ptr<ICE::Asset>& asset, const std::string& path, float t);

    // Drop the cached thumbnail/preview renderers (and their GPU framebuffers) for an asset.
    // Call when an asset is removed or its path changes; otherwise m_renderers grows unbounded
    // and a later asset reusing the path would show the stale preview.
    void evict(const std::string& path) {
        m_renderers.erase("thumb_" + path);
        m_renderers.erase("preview_" + path);
    }

   private:
    // A cached offscreen renderer plus the target it presents into. render() no longer hands back a
    // framebuffer (the pipeline decides what it composites), so each preview drives an explicit
    // render-to-texture target and reads that back for ImGui.
    struct Preview {
        ICE::ForwardRenderer renderer;
        std::shared_ptr<ICE::Framebuffer> target;
        Preview(const std::shared_ptr<ICE::RendererAPI>& api, const std::shared_ptr<ICE::GraphicsFactory>& factory,
                const std::shared_ptr<ICE::GPURegistry>& bank)
            : renderer(api, factory, bank),
              target(factory->createFramebuffer({256, 256, 1})) {
            renderer.resize(256, 256);
        }
    };

    std::unordered_map<std::string, Preview> m_renderers;
    std::shared_ptr<ICE::RendererAPI> m_api;
    std::shared_ptr<ICE::GraphicsFactory> m_factory;
    std::shared_ptr<ICE::GPURegistry> m_bank;
};
