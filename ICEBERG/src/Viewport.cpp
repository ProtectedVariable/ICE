#include "Viewport.h"

#include <ICEMath.h>

#include <iostream>

Viewport::Viewport(const std::shared_ptr<ICE::ICEEngine> &engine, const std::function<void()> &entity_transformed_callback,
                   const std::function<void(ICE::Entity e)> &entity_picked_callback)
    : m_engine(engine),
      m_entity_transformed_callback(entity_transformed_callback),
      m_entity_picked_callback(entity_picked_callback) {
    engine->setRenderFramebufferInternal(true);

    m_picking_frambuffer = engine->getGraphicsFactory()->createFramebuffer({1, 1, 1});

    ui.registerCallback("w_pressed", [this]() { m_engine->getCamera()->forward(camera_delta); });
    ui.registerCallback("s_pressed", [this]() { m_engine->getCamera()->backward(camera_delta); });
    ui.registerCallback("a_pressed", [this]() { m_engine->getCamera()->left(camera_delta); });
    ui.registerCallback("d_pressed", [this]() { m_engine->getCamera()->right(camera_delta); });
    ui.registerCallback("ls_pressed", [this]() { m_engine->getCamera()->up(camera_delta); });
    ui.registerCallback("lc_pressed", [this]() { m_engine->getCamera()->down(camera_delta); });
    ui.registerCallback("mouse_dragged", [this](float dx, float dy) {
        if (!ImGuizmo::IsUsingAny()) {
            m_engine->getCamera()->yaw(dx / 6.0);
            m_engine->getCamera()->pitch(dy / 6.0);
        }
    });
    ui.registerCallback("mouse_clicked", [this](float x, float y) {
        auto fmt = m_engine->getInternalFramebuffer()->getFormat();
        m_picking_frambuffer->bind();
        m_picking_frambuffer->resize(fmt.width, fmt.height);
        m_engine->getApi()->setViewport(0, 0, fmt.width, fmt.height);
        m_engine->getApi()->setClearColor(0, 0, 0, 0);
        m_engine->getApi()->clear();

        auto camera = m_engine->getCamera();
        m_engine->getAssetBank()->getAsset<ICE::Shader>("__ice__picking_shader")->bind();
        m_engine->getAssetBank()->getAsset<ICE::Shader>("__ice__picking_shader")->loadMat4("projection", camera->getProjection());
        m_engine->getAssetBank()->getAsset<ICE::Shader>("__ice__picking_shader")->loadMat4("view", camera->lookThrough());
        auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
        for (auto e : registry->getEntities()) {
            if (registry->entityHasComponent<ICE::RenderComponent>(e) && registry->entityHasComponent<ICE::TransformComponent>(e)) {

                auto tc = registry->getComponent<ICE::TransformComponent>(e);
                auto rc = registry->getComponent<ICE::RenderComponent>(e);
                m_engine->getAssetBank()->getAsset<ICE::Shader>("__ice__picking_shader")->loadMat4("model", tc->getModelMatrix());
                m_engine->getAssetBank()->getAsset<ICE::Shader>("__ice__picking_shader")->loadInt("objectID", e);
                auto model = m_engine->getAssetBank()->getAsset<ICE::Model>(rc->model);
                for (const auto &mesh : model->getMeshes()) {
                    mesh->getVertexArray()->bind();
                    mesh->getVertexArray()->getIndexBuffer()->bind();
                    m_engine->getApi()->renderVertexArray(mesh->getVertexArray());
                }
            }
        }
        auto color = m_picking_frambuffer->readPixel(x, y);
        m_picking_frambuffer->unbind();
        ICE::Entity e = 0;
        e += color.x();
        e += color.y() << 8;
        e += color.z() << 16;
        m_entity_picked_callback(e);
    });
    ui.registerCallback("resize", [this](float width, float height) {
        m_engine->getCamera()->resize(width, height);
        m_engine->getProject()->getCurrentScene()->getRegistry()->getSystem<ICE::RenderSystem>()->setViewport(0, 0, width, height);
    });
    ui.registerCallback("translate_clicked", [this] { m_guizmo_mode = ImGuizmo::OPERATION::TRANSLATE; });
    ui.registerCallback("rotate_clicked", [this] { m_guizmo_mode = ImGuizmo::OPERATION::ROTATE; });
    ui.registerCallback("scale_clicked", [this] { m_guizmo_mode = ImGuizmo::OPERATION::SCALE; });
}

bool Viewport::update() {
    ui.setTexture(static_cast<char *>(0) + m_engine->getInternalFramebuffer()->getTexture());
    ui.render();

    ImGuizmo::Enable(true);
    if (m_selected_entity != 0) {
        auto tc = m_engine->getProject()->getCurrentScene()->getRegistry()->getComponent<ICE::TransformComponent>(m_selected_entity);
        Eigen::Matrix4f delta_matrix;
        delta_matrix.setZero();
        ImGuizmo::Manipulate(m_engine->getCamera()->lookThrough().transpose().data(), m_engine->getCamera()->getProjection().data(), m_guizmo_mode,
                             ImGuizmo::WORLD, tc->getModelMatrix().data(), delta_matrix.data());
        auto deltaT = Eigen::Vector3f(0, 0, 0);
        auto deltaR = Eigen::Vector3f(0, 0, 0);
        auto deltaS = Eigen::Vector3f(0, 0, 0);

        ImGuizmo::DecomposeMatrixToComponents(delta_matrix.data(), deltaT.data(), deltaR.data(), deltaS.data());
        if (m_guizmo_mode == ImGuizmo::TRANSLATE) {
            tc->position() += deltaT;
        } else if (m_guizmo_mode == ImGuizmo::ROTATE) {
            tc->rotation() += deltaR;
        } else if (m_guizmo_mode == ImGuizmo::SCALE) {
            tc->scale() += (deltaS - Eigen::Vector3f(1, 1, 1));
        }
        if (ImGuizmo::IsUsingAny()) {
            m_entity_transformed_callback();
        }
    }
    return m_done;
}

void Viewport::setSelectedEntity(ICE::Entity e) {
    m_selected_entity = e;
}
