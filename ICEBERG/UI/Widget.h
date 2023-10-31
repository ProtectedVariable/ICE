#pragma once

#include <any>
#include <functional>
#include <string>
#include <unordered_map>

class Widget {
   public:
    virtual void render() = 0;

    void registerCallback(const std::string &event, const std::function<void()> &callback) { m_callbacks.try_emplace(event, callback); }
    void callback(const std::string &event) {
        if (m_callbacks.contains(event)) {
            m_callbacks[event]();
        }
    }

    template<typename F>
    void store_lambda(const std::string &key, F &&f) {
        using arg_tuple = typename function_traits<F>::arg_tuple;
        using return_type = typename function_traits<F>::return_type;

        myHash[key] = [f = std::forward<F>(f)](std::any args) -> void {
            if constexpr (std::is_same_v<return_type, void>) {
                std::apply(f, std::any_cast<arg_tuple &>(args));
            }
        };
    }

    template<typename... Args>
    void call(const std::string &key, Args... arg) {
        call_lambda(myHash[key], arg...);
    }

   private:
    std::unordered_map<std::string, std::function<void(std::any)>> myHash;

    template<typename T>
    struct function_traits : public function_traits<decltype(&T::operator())> {};

    template<typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType (ClassType::*)(Args...) const> {
        using arg_tuple = std::tuple<Args...>;
        using return_type = ReturnType;
    };

    template<typename... Args>
    void call_lambda(const std::function<void(std::any)> &f, Args &&...args) {
        f(std::make_tuple(std::forward<Args>(args)...));
    }

   protected:
    std::unordered_map<std::string, std::function<void()>> m_callbacks;
};