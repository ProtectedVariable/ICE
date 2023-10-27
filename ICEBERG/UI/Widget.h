#pragma once

#include <functional>
#include <string>
#include <unordered_map>

class Widget {
   public:
    virtual void render() = 0;

    void registerCallback(const std::string& event, const std::function<void()>& callback) { m_callbacks.try_emplace(event, callback); }
    void callback(const std::string& event) {
        if (m_callbacks.contains(event)) {
            m_callbacks[event]();
        }
    }

   protected:
    std::unordered_map<std::string, std::function<void()>> m_callbacks;
};
