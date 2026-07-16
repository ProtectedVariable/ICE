#pragma once

#include "OrthographicCamera.h"
#include "UIElement.h"
#include "Window.h"

namespace ICE {
class UI {
 public:
  UI(const std::shared_ptr<Window> &window, const std::shared_ptr<RendererAPI> &api)
      : m_window(window),
        m_api(api),
        m_camera(0, window->getSize().first, 0, window->getSize().second, -1, 1) {}

  void render() {
    auto [w, h] = m_window->getSize();
    m_api->setDepthTest(false);
    //TODO: Orthographic camera should have a setBounds method rather than reconstructing
    m_camera = OrthographicCamera(0, w, 0, h, -1, 1);
    for (const auto &element : m_elements) {
      if (element->isVisible()) {
        element->render({0, 0, (float) w, (float) h}, m_camera.getProjection(), m_api);
      }
    }
  }

  template<typename T>
  T *getElement(const std::string &name) {
    for (const auto &elem : m_elements) {
      if (auto result = getSubElement<T>(name, elem); result) {
        return result;
      }
    }
    return nullptr;
  }

  template<typename T>
  T *getSubElement(const std::string &name, const std::unique_ptr<UIElement> &root) {
    auto pos = name.find_first_of('.', 0);
    T *ret = nullptr;
    if (pos != std::string::npos) {
      auto prefix = name.substr(0, pos);
      if (prefix == root->getName()) {
        for (const auto &child : root->getChildren()) {
          if (auto result = getSubElement<T>(name.substr(pos + 1), child); result) {
            return result;
          }
        }
      }
    } else {
      if (name == root->getName()) {
        ret = dynamic_cast<T *>(root.get());
      }
    }
    return ret;
  }

 protected:
  std::shared_ptr<Window> m_window;
  std::shared_ptr<RendererAPI> m_api;
  OrthographicCamera m_camera;
  std::vector<std::unique_ptr<UIElement>> m_elements;
};
}  // namespace ICE