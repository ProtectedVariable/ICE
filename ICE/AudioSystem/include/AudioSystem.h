#pragma once

#include <AudioEngine.h>
#include <AudioListenerComponent.h>
#include <AudioSourceComponent.h>
#include <Registry.h>
#include <System.h>
#include <TransformComponent.h>

#include <memory>

namespace ICE {

// Drives positional audio from the ECS: pushes the listener's world pose to the audio engine, then
// reconciles every AudioSourceComponent's authored state against its live voice.
//
// Runs at AudioSystemOrder (350) -- after SceneGraphSystem (300), so every world matrix it reads is
// final for the frame, and before RenderSystem (400), so a frame's audio and visuals derive from
// the same state.
//
// Listener selection, in order:
//   1. an entity with an active AudioListenerComponent (explicit intent wins), else
//   2. the listener entity set via setListenerEntity() -- how the engine passes the scene's active
//      camera, so a single-viewpoint game needs no setup at all, else
//   3. no listener: spatial sources are still positioned, relative to the origin.
class AudioSystem : public System {
   public:
    AudioSystem(const std::shared_ptr<Registry>& registry, AudioEngine* audio);

    void update(double delta) override;
    void onEntityRemoved(Entity e) override;

    int updateOrder() const override { return AudioSystemOrder; }

    // Fallback listener used when no entity carries an AudioListenerComponent. The engine points
    // this at the scene's active camera entity on activation. NULL_ENTITY disables the fallback.
    void setListenerEntity(Entity e) { m_fallback_listener = e; }
    Entity getListenerEntity() const { return m_fallback_listener; }

    // The entity that actually drove the listener last update (NULL_ENTITY if none did). Exposed
    // for the editor's audio panel and for tests.
    Entity getActiveListener() const { return m_active_listener; }

    std::vector<Signature> getSignatures(const ComponentManager& comp_manager) const override {
        // Sources need a transform to be positioned; listeners need one to be oriented. Two
        // signatures rather than one, so an entity carrying either is tracked.
        Signature source_sig;
        source_sig.set(comp_manager.getComponentType<AudioSourceComponent>());
        source_sig.set(comp_manager.getComponentType<TransformComponent>());

        Signature listener_sig;
        listener_sig.set(comp_manager.getComponentType<AudioListenerComponent>());
        listener_sig.set(comp_manager.getComponentType<TransformComponent>());

        return {source_sig, listener_sig};
    }

   private:
    // Find and push the listener pose. Returns the entity used, or NULL_ENTITY.
    Entity updateListener(double delta);

    // Reconcile one source's authored state with its live voice.
    void updateSource(Entity e, AudioSourceComponent& source, TransformComponent& transform, double delta);

    // Start a voice for `source`, recording its handle in the component.
    void startVoice(Entity e, AudioSourceComponent& source, const Eigen::Vector3f& world_position);

    // Translate the component's authored fields into device parameters.
    VoiceParams buildParams(const AudioSourceComponent& source, const Eigen::Vector3f& world_position,
                            const Eigen::Vector3f& velocity) const;

    static VoiceHandle handleOf(const AudioSourceComponent& source) {
        return VoiceHandle{source.voice_index, source.voice_generation};
    }
    static void setHandle(AudioSourceComponent& source, VoiceHandle handle) {
        source.voice_index = handle.index;
        source.voice_generation = handle.generation;
    }
    static void clearHandle(AudioSourceComponent& source) { setHandle(source, VoiceHandle{}); }

    // Per-frame world position delta, in units/second, for Doppler. Returns zero on the first frame
    // a thing is seen (no previous sample) and whenever delta is degenerate -- a spurious huge
    // velocity would produce an audible pitch glitch on spawn.
    static Eigen::Vector3f velocityFrom(const Eigen::Vector3f& current, Eigen::Vector3f& last, bool& has_last, double delta);

    // Non-owning: the Registry owns this system, so a shared_ptr here would form an ownership
    // cycle (same reason SceneGraphSystem holds a raw Scene*).
    Registry* m_registry = nullptr;
    AudioEngine* m_audio = nullptr;

    Entity m_fallback_listener = NULL_ENTITY;
    Entity m_active_listener = NULL_ENTITY;
    bool m_warned_multiple_listeners = false;

    // Previous listener position when the listener is the camera-entity fallback. An explicit
    // AudioListenerComponent caches this in the component instead; the fallback entity has no
    // audio component to hold it, so it lives here.
    Eigen::Vector3f m_fallback_last_position = Eigen::Vector3f::Zero();
    bool m_fallback_has_last_position = false;
};

}  // namespace ICE
