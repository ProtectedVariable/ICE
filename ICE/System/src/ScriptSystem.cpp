#include "ScriptSystem.h"

#include <NativeScript.h>  // full type: the system injects context into and reads flags off instances

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
    // Inject the behaviour context before onCreate so scripts can already use
    // self()/transform()/scene()/input()/time() there.
    instance->m_entity = e;
    instance->m_registry = m_registry;
    instance->m_scene = m_scene;
    instance->m_input = m_input;
    instance->m_time = m_elapsed;
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
    m_elapsed += delta;
    // Snapshot the membership: a script's onUpdate may add or remove components, which mutates
    // the `entities` set mid-iteration.
    std::vector<Entity> current(entities.begin(), entities.end());
    for (Entity e : current) {
        if (NativeScript* script = ensureInstance(e)) {
            script->m_time = m_elapsed;  // refresh for instances created on a previous frame
            script->onUpdate(delta);
            if (script->m_destroyed) {
                m_pending_destroy.push_back(e);
            }
        }
    }
    // Deferred self-destruction: removing an entity mid-onUpdate would delete the running script
    // (and fire its onDestroy) under its own feet, so destroy() only set a flag. Do the real
    // removal here, after the script pass -- removeEntity() drives onEntityRemoved() which fires
    // onDestroy and erases the instance.
    if (!m_pending_destroy.empty()) {
        for (Entity e : m_pending_destroy) {
            m_registry->removeEntity(e);
        }
        m_pending_destroy.clear();
    }
}
}  // namespace ICE
