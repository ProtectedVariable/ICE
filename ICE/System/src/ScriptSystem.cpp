#include "ScriptSystem.h"

namespace ICE {

NativeScript* ScriptSystem::ensureInstance(Entity e) {
    auto it = m_instances.find(e);
    if (it != m_instances.end()) {
        return it->second.get();
    }
    auto* nsc = m_registry->tryGetComponent<NativeScriptComponent>(e);
    if (nsc == nullptr || !nsc->instantiate) {
        return nullptr;  // no script bound
    }
    auto instance = nsc->instantiate();
    instance->m_entity = e;
    instance->m_registry = m_registry;
    NativeScript* raw = instance.get();
    m_instances.emplace(e, std::move(instance));
    raw->onCreate();
    return raw;
}

void ScriptSystem::onEntityAdded(Entity e) {
    // Idempotent: onEntityAdded may fire again when unrelated components change, so only the
    // first call (no existing instance) actually instantiates.
    ensureInstance(e);
}

void ScriptSystem::onEntityRemoved(Entity e) {
    auto it = m_instances.find(e);
    if (it == m_instances.end()) {
        return;  // not a scripted entity (or already torn down) -- idempotent no-op
    }
    it->second->onDestroy();
    m_instances.erase(it);
}

void ScriptSystem::update(double delta) {
    // Snapshot the membership: a script's onUpdate may add or remove components, which mutates
    // the `entities` set mid-iteration.
    std::vector<Entity> current(entities.begin(), entities.end());
    for (Entity e : current) {
        if (NativeScript* script = ensureInstance(e)) {
            script->onUpdate(delta);
        }
    }
}
}  // namespace ICE
