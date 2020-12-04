//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetViewPane.h"
#include <ImGUI/imgui.h>

namespace ICE {
    void AssetViewPane::render() {
        ImGui::Begin("Asset View");
        ImVec2 wsize = ImGui::GetWindowContentRegionMax();
        ImVec2 pos = ImGui::GetCursorPos();
        wsize = ImVec2(wsize.x - pos.x, wsize.y - pos.y);
        viewFB->bind();
        viewFB->resize(wsize.x, wsize.y);
        engine->getApi()->setViewport(0, 0, wsize.x, wsize.y);
        engine->getApi()->clear();
        Shader* shader = engine->getAssetBank()->getShader("__ice__phong_shader");
        shader->bind();
        camera->setParameters({60, wsize.x / wsize.y, 0.01f, 1000 }, Perspective);
        shader->loadMat4("projection", camera->getProjection());
        shader->loadMat4("view", camera->lookThrough());
        shader->loadMat4("model", Eigen::Matrix4f().setIdentity());
        engine->getApi()->renderVertexArray(engine->getAssetBank()->getMesh("__ice__sphere")->getVertexArray());
        viewFB->unbind();
        ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
    }

    AssetViewPane::AssetViewPane(ICEEngine *engine) : engine(engine), viewFB(Framebuffer::Create({400, 400, 1})) {
        camera = new Camera({{60, 16.f / 9.f, 0.01f, 1000 }, Perspective });
        camera->getPosition().y() = 1;
        camera->getPosition().z() = 2;
        camera->getRotation().x() = -30;
    }
}
