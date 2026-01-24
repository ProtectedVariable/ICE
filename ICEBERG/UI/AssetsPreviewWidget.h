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
    AssetsPreviewWidget() = default;

    void render() override { ImGui::Image(m_texture, {256, 256}, ImVec2(0, 1), ImVec2(1, 0)); }

    void setTexture(void* tex) { m_texture = tex; }

   private:
    void* m_texture = nullptr;
};