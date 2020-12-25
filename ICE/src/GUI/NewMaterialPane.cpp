//
// Created by Thomas Ibanez on 21.12.20.
//

#include "NewMaterialPane.h"
#include <Core/ICEEngine.h>
#include <Util/ICEMath.h>

#define ICE_NEWMAT_PICKER_WIDTH 150

namespace ICE {
    bool NewMaterialPane::render() {
        auto ret = true;
        ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoOptions;
        ImGui::Begin(editMode ? "Edit Material" : "Create New Material", 0, ImGuiWindowFlags_NoCollapse);
        ImGui::Text("Name");
        ImGui::SameLine();
        char buffer[512];
        strcpy(buffer, name.c_str());
        ImGui::InputText("##Name", buffer, 512);
        name = std::string(buffer);

        //Colors
        ImGui::Text("Albedo");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
        ImGui::ColorPicker3("##NewMaterialAlbedo", albedo.data(), flags);
        ImGui::SameLine();
        ImGui::Text("Specular");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
        ImGui::ColorPicker3("##NewMaterialSpecular", specular.data(), flags);
        ImGui::SameLine();
        ImGui::Text("Ambient");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
        ImGui::ColorPicker3("##NewMaterialAmbient", ambient.data(), flags);


        //Maps
        auto textures = std::vector<const char*>(engine->getAssetBank()->getTextures().size() + 1);
        textures[0] = "None";
        int i = 1;
        int selected[4] = {0,0,0,0};
        const char* mapNames[4] =  {"Diffuse", "Specular", "Ambient", "Normal"};
        for(const auto &t : engine->getAssetBank()->getTextures()) {
            textures[i++] = t.first.c_str();
            if(t.second == diffuseMap) {
                selected[0] = i-1;
            }
            if(t.second == specularMap) {
                selected[1] = i-1;
            }
            if(t.second == ambientMap) {
                selected[2] = i-1;
            }
            if(t.second == normalMap) {
                selected[3] = i-1;
            }
        }
        ImGui::Image(diffuseMap == nullptr ? 0 : diffuseMap->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SameLine();
        ImGui::Image(specularMap == nullptr ? 0 : specularMap->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SameLine();
        ImGui::Image(ambientMap == nullptr ? 0 : ambientMap->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SameLine();
        ImGui::Image(normalMap == nullptr ? 0 : normalMap->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH), ImVec2(0, 1), ImVec2(1, 0));

        for(int i = 0; i < 4; i++) {
            ImGui::Text("%s Map", mapNames[i]);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
            ImGui::Combo(("##NewMaterial"+std::string(mapNames[i])+"Map").c_str(), &selected[i], textures.data(), engine->getAssetBank()->getTextures().size() + 1, 10);
            if(i < 3)
                ImGui::SameLine();
        }

        for(int i = 0; i < 4; i++) {
            const Texture** map = nullptr;
            switch(i) {
                case 0:
                    map = &diffuseMap;
                    break;
                case 1:
                    map = &specularMap;
                    break;
                case 2:
                    map = &ambientMap;
                    break;
                case 3:
                    map = &normalMap;
                    break;
            }
            if(selected[i] > 0) {
                *map = engine->getAssetBank()->getTexture(textures[selected[i]]);
            } else {
                *map = nullptr;
            }
        }

        //Alpha
        ImGui::Text("Alpha");
        ImGui::SameLine();
        ImGui::SliderFloat("##NewMaterialAlpha", &alpha, 0, 100);

        //Preview
        ImVec2 wsize = ImGui::GetWindowContentRegionMax();
        ImVec2 pos = ImGui::GetCursorPos();
        wsize = ImVec2(wsize.x - pos.x, wsize.y - pos.y - 30);

        Material mat = makeMaterial();
        auto scene = Scene("__ice__newmaterial_scene");

        auto sphere = Entity();
        auto rcSphere = RenderComponent(engine->getAssetBank()->getMesh("__ice__sphere"), &mat, engine->getAssetBank()->getShader("__ice__phong_shader"));
        auto tcSphere = TransformComponent();
        sphere.addComponent(&rcSphere);
        sphere.addComponent(&tcSphere);
        scene.addEntity("sphere", &sphere);

        auto light = Entity();
        auto lcLight = LightComponent(PointLight, Eigen::Vector3f(1,1,1));
        auto tcLight = TransformComponent(Eigen::Vector3f(10,20,10), Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1));
        light.addComponent(&lcLight);
        light.addComponent(&tcLight);
        scene.addEntity("light", &light);

        camera.setParameters({50, wsize.x / wsize.y, 0.01f, 1000});
        renderer.setTarget(viewFB);
        renderer.submitScene(&scene);
        renderer.prepareFrame(camera);
        renderer.resize(wsize.x, wsize.y);

        tcSphere.getRotation()->y() += y++;
        renderer.render();
        renderer.endFrame();

        ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        if(ImGui::Button(editMode ? "Edit" : "Add")) {
            ret = false;
        }
        ImGui::End();
        return ret;
    }

    NewMaterialPane::NewMaterialPane(ICEEngine* engine): engine(engine), name("new_material"),
                                                         viewFB(Framebuffer::Create({200, 200, 1})),
                                                         camera(Camera({{60, 16.f / 9.f, 0.01f, 1000 }, Perspective })),
                                                         renderer(ForwardRenderer()){
        reset();
        camera.getPosition().y() = 1;
        camera.getPosition().z() = 2;
        camera.getRotation().x() = -30;
        renderer.initialize(RendererConfig());
    }

    void NewMaterialPane::reset() {
        albedo = Eigen::Vector3f(1,1,1);
        specular = Eigen::Vector3f(1,1,1);
        ambient = Eigen::Vector3f(.1f,.1f,.1f);
        alpha = 1.f;
        diffuseMap = normalMap = ambientMap = specularMap = nullptr;
        editMode = false;
        name = "newmaterial";
    }

    void NewMaterialPane::build() {
        auto mtl = Material(albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, normalMap);
        if(!editMode) {
            auto nm = new Material();
            *nm = mtl;
            engine->getAssetBank()->addMaterial(name, nm);
        } else {
            std::string newName = oldname;
            if(engine->getAssetBank()->renameAsset(oldname, name)) {
                newName = name;
            }
            *engine->getAssetBank()->getMaterial(newName) = mtl;
        }
    }

    void NewMaterialPane::edit(const std::string& name, Material& m) {
        albedo = m.getAlbedo();
        specular = m.getSpecular();
        ambient = m.getAmbient();
        alpha = m.getAlpha();
        diffuseMap = m.getDiffuseMap();
        specularMap = m.getSpecularMap();
        ambientMap = m.getAmbientMap();
        normalMap = m.getNormalMap();
        editMode = true;
        this->name = name;
        this->oldname = name;
    }

    Material NewMaterialPane::makeMaterial() {
        return Material(albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, normalMap);
    }
}