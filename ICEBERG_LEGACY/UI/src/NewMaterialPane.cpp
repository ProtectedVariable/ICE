//
// Created by Thomas Ibanez on 21.12.20.
//

#include "NewMaterialPane.h"

#include <ICEEngine.h>
#include <ICEMath.h>
#include <ImGUI/imgui.h>
#include <ImGUI/imgui_internal.h>


#define ICE_NEWMAT_PICKER_WIDTH 150

namespace ICE {
bool NewMaterialPane::render() {
    auto ret = true;
    ImGuiColorEditFlags flags = ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoOptions;
    ImGui::Begin(editMode ? "Edit Material" : "Create New Material", 0, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Name");
    ImGui::SameLine();
    char buffer[512];
    strcpy(buffer, name.getName().c_str());
    ImGui::InputText("##Name", buffer, 512);
    AssetPath newName = AssetPath(name);
    newName.setName(buffer);
    engine->getAssetBank()->renameAsset(name, newName);
    name = newName;

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
    auto textures = std::vector<const char*>(engine->getAssetBank()->getAll<Texture>().size() + 1);
    textures[0] = "None";
    int i = 1;
    int selected[4] = {0, 0, 0, 0};
    const char* mapNames[4] = {"Diffuse", "Specular", "Ambient", "Normal"};
    for (const auto& t : engine->getAssetBank()->getAll<Texture>()) {
        textures[i++] = engine->getAssetBank()->getName(t.first).toString().c_str();
        if (t.first == diffuseMap) {
            selected[0] = i - 1;
        }
        if (t.first == specularMap) {
            selected[1] = i - 1;
        }
        if (t.first == ambientMap) {
            selected[2] = i - 1;
        }
        if (t.first == normalMap) {
            selected[3] = i - 1;
        }
    }
    ImGui::Image(diffuseMap == NO_ASSET_ID ? 0 : engine->getAssetBank()->getAsset<Texture>(diffuseMap)->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH),
                 ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine(0, ICE_NEWMAT_PICKER_WIDTH / 2);
    ImGui::Image(specularMap == NO_ASSET_ID ? 0 : engine->getAssetBank()->getAsset<Texture>(specularMap)->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH),
                 ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine(0, ICE_NEWMAT_PICKER_WIDTH / 2);
    ImGui::Image(ambientMap == NO_ASSET_ID ? 0 : engine->getAssetBank()->getAsset<Texture>(ambientMap)->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH),
                 ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine(0, ICE_NEWMAT_PICKER_WIDTH / 2);
    ImGui::Image(normalMap == NO_ASSET_ID ? 0 : engine->getAssetBank()->getAsset<Texture>(normalMap)->getTexture(), ImVec2(ICE_NEWMAT_PICKER_WIDTH, ICE_NEWMAT_PICKER_WIDTH),
                 ImVec2(0, 1), ImVec2(1, 0));

    for (int i = 0; i < 4; i++) {
        ImGui::Text("%s Map", mapNames[i]);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(ICE_NEWMAT_PICKER_WIDTH);
        ImGui::Combo(("##NewMaterial" + std::string(mapNames[i]) + "Map").c_str(), &selected[i], textures.data(), engine->getAssetBank()->getAll<Texture>().size() + 1, 10);
        if (i < 3)
            ImGui::SameLine();
    }

    //Alpha
    ImGui::Text("Alpha");
    ImGui::SameLine();
    ImGui::SliderFloat("##NewMaterialAlpha", &alpha, 0, 100);

    //Preview
    ImVec2 wsize = ImGui::GetWindowContentRegionMax();
    ImVec2 pos = ImGui::GetCursorPos();
    wsize = ImVec2(wsize.x - pos.x, wsize.y - pos.y - 30);

    *engine->getAssetBank()->getAsset<Material>(name) = Material(albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, normalMap);
    AssetUID mat = engine->getAssetBank()->getUID(name);
    auto scene = Scene("__ice__newmaterial_scene", new Registry());
    /*
        auto sphere = Entity();
        auto rcSphere = RenderComponent(engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Mesh>("__ice__sphere")), mat, engine->getAssetBank()->getUID(AssetPath::WithTypePrefix<Shader>("__ice__phong_shader")));
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

        if(wsize.x > 0 && wsize.y > 0) {
            camera.setParameters({50, wsize.x / wsize.y, 0.01f, 1000});
            renderer.setTarget(viewFB);
            renderer.submitScene(&scene);
            renderer.prepareFrame(camera);

            renderer.resize(wsize.x, wsize.y);

            tcSphere.getRotation()->y() += y++;
            renderer.render();
            renderer.endFrame();

            ImGui::Image(viewFB->getTexture(), wsize, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false); //TODO: engine->getAssetBank()->nameInUse(name) && !editMode
        if(ImGui::Button(editMode ? "Edit" : "Add")) {
            ret = false;
        }
        */
    ImGui::PopItemFlag();
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        ret = false;
        *engine->getAssetBank()->getAsset<Material>(name) = backup;
        engine->getAssetBank()->renameAsset(name, nameBackup);
        if (!editMode) {
            engine->getAssetBank()->removeAsset(name);
        }
        canceled = true;
    }
    ImGui::End();
    return ret;
}

NewMaterialPane::NewMaterialPane(ICEEngine* engine)
    : engine(engine),
      name("new_material"),
      nameBackup("new_material"),
      viewFB(Framebuffer::Create({200, 200, 1})),
      camera(Camera({{60, 16.f / 9.f, 0.01f, 1000}, Perspective})),
      renderer(ForwardRenderer()) {
    camera.getPosition().y() = 1;
    camera.getPosition().z() = 2;
    camera.getRotation().x() = -30;
}

void NewMaterialPane::reset() {
    albedo = Eigen::Vector3f(1, 1, 1);
    specular = Eigen::Vector3f(1, 1, 1);
    ambient = Eigen::Vector3f(.1f, .1f, .1f);
    alpha = 1.f;
    diffuseMap = normalMap = ambientMap = specularMap = NO_ASSET_ID;
    editMode = false;
    name = AssetPath::WithTypePrefix<Material>("new_material");
    nameBackup = name;
    canceled = false;
    engine->getAssetBank()->addResource(name, new Resource(new Material(albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, normalMap), {}));
}

void NewMaterialPane::build() {
    if (canceled)
        return;
    auto mtl = Material(albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, normalMap);
}

void NewMaterialPane::edit(AssetUID selectedAsset, Material& m) {
    albedo = m.getAlbedo();
    specular = m.getSpecular();
    ambient = m.getAmbient();
    alpha = m.getAlpha();
    diffuseMap = m.getDiffuseMap();
    specularMap = m.getSpecularMap();
    ambientMap = m.getAmbientMap();
    normalMap = m.getNormalMap();
    editMode = true;
    this->name = engine->getAssetBank()->getName(selectedAsset);
    this->backup = m;
}

Material NewMaterialPane::makeMaterial() {
    return Material(albedo, specular, ambient, alpha, diffuseMap, specularMap, ambientMap, normalMap);
}

void NewMaterialPane::initialize() {
    renderer.initialize(RendererConfig(), engine->getAssetBank());
}
}  // namespace ICE