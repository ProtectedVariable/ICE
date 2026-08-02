#pragma once

#include <string>

namespace ICE {
class Registry;

// Seam for an embedded scripting VM (Lua, Wren, ...) with hot-reload, complementing the
// compile-time NativeScript. A backend binds the ECS to the VM and drives per-frame script logic;
// hot-reload is expressed by (re)loading a script by path at runtime.
//
// Interface-first: there is no in-tree VM yet. This defines the contract a backend implements and
// the engine drives, so adding a VM later is a plug-in rather than an engine rewrite.
class IScriptingBackend {
   public:
    virtual ~IScriptingBackend() = default;

    // Called once when the backend is attached, giving the VM access to the ECS to bind.
    virtual void initialize(Registry& registry) = 0;

    // (Re)load a script from disk. Calling it again on the same path is the hot-reload path.
    virtual void loadScript(const std::string& path) = 0;

    // Drive script logic for the frame. The engine runs this alongside the native ScriptSystem.
    virtual void update(double delta) = 0;
};

}  // namespace ICE
