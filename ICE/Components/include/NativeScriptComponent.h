#pragma once

#include <functional>
#include <memory>

#include "NativeScript.h"

namespace ICE {

// Attaches native game logic to an entity. bind<T>() only records how to construct the script;
// the ScriptSystem owns the live instance (keyed by entity) and drives its lifecycle. The
// component therefore holds nothing but a factory, so it stays trivially copyable -- entity
// duplication just copies the factory and the duplicate gets its own fresh instance.
struct NativeScriptComponent {
    std::function<std::shared_ptr<NativeScript>()> instantiate;

    template<typename T, typename... Args>
    void bind(Args... args) {
        instantiate = [args...]() { return std::static_pointer_cast<NativeScript>(std::make_shared<T>(args...)); };
    }
};
}  // namespace ICE
