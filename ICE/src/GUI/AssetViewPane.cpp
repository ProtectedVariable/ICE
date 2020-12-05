//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetViewPane.h"
#include <ImGUI/imgui.h>
#include <Util/ICEMath.h>
#include <Graphics/Shader.h>
#include <Core/ICEEngine.h>

namespace ICE {
    int y = 45;
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
        camera.setParameters({60, wsize.x / wsize.y, 0.01f, 1000});
        shader->loadMat4("projection", camera.getProjection());
        shader->loadMat4("view", camera.lookThrough());
        shader->loadMat4("model", rotationMatrix(Eigen::Vector3f(0, y++, 0)));
        //TODO: Differentiate preview depending on if the selected asset is a mesh or material
        engine->getApi()->renderVertexArray(engine->getAssetBank()->getMesh(*selectedAsset)->getVertexArray());
        viewFB->unbind();
        ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
    }

    AssetViewPane::AssetViewPane(ICEEngine *engine, std::string* selectedAsset) : engine(engine), selectedAsset(selectedAsset),
                                                                                    viewFB(Framebuffer::Create({400, 400, 1})),
                                                                                    camera(Camera({{60, 16.f / 9.f, 0.01f, 1000 }, Perspective })) {
        camera.getPosition().y() = 1;
        camera.getPosition().z() = 2;
        camera.getRotation().x() = -30;
    }
}
