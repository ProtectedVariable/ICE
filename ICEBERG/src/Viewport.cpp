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
        auto shader = m_engine->getGPURegistry()->getShader(ICE::AssetPath::WithTypePrefix<ICE::Shader>("__ice__picking_shader"));

        shader->bind();

        auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
        for (auto e : registry->getEntities()) {
            if (registry->entityHasComponent<ICE::RenderComponent>(e) && registry->entityHasComponent<ICE::TransformComponent>(e)) {

                auto tc = registry->getComponent<ICE::TransformComponent>(e);
                auto rc = registry->getComponent<ICE::RenderComponent>(e);
                shader->loadMat4("model", tc->getWorldMatrix());
                shader->loadInt("objectID", e);
                auto mesh = m_engine->getGPURegistry()->getMesh(rc->mesh);
                if (mesh) {
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
        if (e != 0)
            m_entity_picked_callback(e);
    });
    ui.registerCallback("resize", [this](float width, float height) {
        m_engine->getCamera()->resize(width, height);
        m_engine->getProject()->getCurrentScene()->getRegistry()->getSystem<ICE::RenderSystem>()->setViewport(0, 0, width, height);
    });
    ui.registerCallback("translate_clicked", [this] { m_guizmo_mode = ImGuizmo::OPERATION::TRANSLATE; });
    ui.registerCallback("rotate_clicked", [this] { m_guizmo_mode = ImGuizmo::OPERATION::ROTATE; });
    ui.registerCallback("scale_clicked", [this] { m_guizmo_mode = ImGuizmo::OPERATION::SCALE; });

    ui.registerCallback("spawnTree", [this](char* path) {
        auto uid = m_engine->getProject()->getAssetBank()->getUID(std::string(path));
        if (uid == NO_ASSET_ID)
            return;
        ICE::Entity e = m_engine->getProject()->getCurrentScene()->spawnTree(uid, m_engine->getAssetBank());
        m_entity_picked_callback(e);
    });
}

bool Viewport::update() {
    ui.setTexture(static_cast<char *>(0) + m_engine->getInternalFramebuffer()->getTexture());
    ui.render();

    if (m_selected_entity != 0) {
        auto registry = m_engine->getProject()->getCurrentScene()->getRegistry();
        auto tc = registry->getComponent<ICE::TransformComponent>(m_selected_entity);

        ICE::Entity parentID = m_engine->getProject()->getCurrentScene()->getGraph()->getParentID(m_selected_entity);
        Eigen::Matrix4f parentWorldMatrix = Eigen::Matrix4f::Identity();

        if (parentID != 0) {
            auto ptc = registry->getComponent<ICE::TransformComponent>(parentID);
            parentWorldMatrix = ptc->getWorldMatrix();
        }

        Eigen::Matrix4f currentWorldMatrix = tc->getWorldMatrix().eval();

        ImGuizmo::Manipulate(m_engine->getCamera()->lookThrough().data(), m_engine->getCamera()->getProjection().data(), m_guizmo_mode,
                             ImGuizmo::LOCAL, currentWorldMatrix.data());

        if (ImGuizmo::IsUsing()) {
            Eigen::Matrix4f newLocalMatrix = parentWorldMatrix.inverse() * currentWorldMatrix;

            Eigen::Vector3f pos = newLocalMatrix.block<3, 1>(0, 3);

            Eigen::Vector3f sca;
            sca.x() = newLocalMatrix.block<3, 1>(0, 0).norm();
            sca.y() = newLocalMatrix.block<3, 1>(0, 1).norm();
            sca.z() = newLocalMatrix.block<3, 1>(0, 2).norm();

            Eigen::Matrix3f rotMat = newLocalMatrix.block<3, 3>(0, 0);
            rotMat.col(0) /= sca.x();
            rotMat.col(1) /= sca.y();
            rotMat.col(2) /= sca.z();

            Eigen::Quaternionf rot(rotMat);

            tc->setPosition(pos);
            tc->setRotation(rot);
            tc->setScale(sca);

            m_entity_transformed_callback();
        }
    }
    return m_done;
}

void Viewport::setSelectedEntity(ICE::Entity e) {
    m_selected_entity = e;
}
