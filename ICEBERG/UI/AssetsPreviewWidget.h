#pragma once

#include <ForwardRenderer.h>
#include <PerspectiveCamera.h>
#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

class AssetsPreviewWidget : public Widget {
   public:
    explicit AssetsPreviewWidget(const ICE::ForwardRenderer& renderer) : m_renderer(renderer) {}

    void render() override {
        auto fb = m_renderer.render();
        ImGui::Image(fb->getTexture(), {256, 256});
    }

    void setDrawable(const ICE::Drawable& drawable) { m_drawable = drawable; }

   private:
    ICE::ForwardRenderer m_renderer;
    ICE::Drawable m_drawable{nullptr, nullptr, nullptr, {}, {}};
    ICE::Light m_light;
    ICE::PerspectiveCamera m_camera{60, 1.0f, 0.1f, 100.f};
};