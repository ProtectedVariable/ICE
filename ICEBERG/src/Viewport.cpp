#include "Viewport.h"

#include <ICEMath.h>

#include <iostream>

Viewport::Viewport(const std::shared_ptr<ICE::ICEEngine> &engine) : m_engine(engine) {
    engine->setRenderFramebufferInternal(true);

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
    }
    return m_done;
}

void Viewport::setSelectedEntity(ICE::Entity e) {
    m_selected_entity = e;
}
