//
// Created by Thomas Ibanez on 04.12.20.
//

#include "AssetContentPane.h"
#include <ImGUI/imgui.h>
#include <Util/ICEMath.h>

#define ICE_THUMBNAIL_SIZE (64)
#define ICE_MAX_THUMBNAILS (128)

namespace ICE {

    void AssetContentPane::render() {
        ImGui::Begin("Asset Content");
        ImGui::Columns(6);
        int i = 0;
        if(*selectedDir == 0) {

            for(const auto& m : engine->getAssetBank()->getMeshes()) {
                thumbnailFBO[i]->bind();
                engine->getApi()->setViewport(0, 0, ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE);
                engine->getApi()->clear();
                Shader* shader = engine->getAssetBank()->getShader("__ice__normal_shader");
                shader->bind();
                shader->loadMat4("projection", camera->getProjection());
                shader->loadMat4("view", camera->lookThrough());
                shader->loadMat4("model", rotationMatrix(Eigen::Vector3f(0, 45, 0)));
                engine->getApi()->renderVertexArray(m.second->getVertexArray());
                engine->getApi()->flush();
                engine->getApi()->finish();
                thumbnailFBO[i]->unbind();
                ImGui::Image(thumbnailFBO[i]->getTexture(), ImVec2(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE), ImVec2(0, 1), ImVec2(1, 0));
                if(ImGui::IsItemClicked(0)) {
                    *selectedAsset = m.first;
                }
                if(*selectedAsset == m.first) {
                    ImGui::TextColored(ImVec4(0,0.8f,1,1), "%s", m.first.c_str());
                } else {
                    ImGui::Text("%s", m.first.c_str());
                }
                ImGui::NextColumn();
                i++;
            }
        } else if(*selectedDir == 1) {
            for(const auto& m : engine->getAssetBank()->getMaterials()) {
                thumbnailFBO[i]->bind();
                engine->getApi()->setViewport(0, 0, ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE);
                engine->getApi()->clear();
                Shader* shader = engine->getAssetBank()->getShader("__ice__phong_shader");
                shader->bind();
                shader->loadMat4("projection", camera->getProjection());
                shader->loadMat4("view", camera->lookThrough());
                shader->loadMat4("model", rotationMatrix(Eigen::Vector3f(0, 45, 0)));
                //TODO: When the shader is done, upload the material data
                engine->getApi()->renderVertexArray(engine->getAssetBank()->getMesh("__ice__sphere")->getVertexArray());
                engine->getApi()->flush();
                engine->getApi()->finish();
                thumbnailFBO[i]->unbind();
                ImGui::Image(thumbnailFBO[i]->getTexture(), ImVec2(ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::Text("%s", m.first.c_str());
                ImGui::NextColumn();
            }
        } else if(*selectedDir == 2) {
            for(const auto& m : engine->getAssetBank()->getShaders()) {
                //TODO: Add file icon
                ImGui::Text("%s", m.first.c_str());
                ImGui::NextColumn();
            }
        }
        ImGui::End();
    }

    AssetContentPane::AssetContentPane(const int *selectedDir, ICEEngine *engine, std::string* selectedAsset) : selectedDir(selectedDir),
                                                                                    engine(engine), selectedAsset(selectedAsset),
                                                                                    camera(new Camera({{60, 1.f, 0.01f, 1000 }, Perspective})) {
        camera->getPosition().z() = 3;
        camera->getPosition().y() = 2;
        camera->getRotation().x() = -30;
        this->thumbnailFBO = new Framebuffer*[ICE_MAX_THUMBNAILS];
        for(int i = 0; i < ICE_MAX_THUMBNAILS; i++) {
            this->thumbnailFBO[i] = Framebuffer::Create({ICE_THUMBNAIL_SIZE, ICE_THUMBNAIL_SIZE, 1});
        }
    }
}