#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <utility>
#include <vector>

namespace ICE {

// A lightweight, typed handle: a slot index plus a generation. `generation == 0` is the null
// handle. The generation is what makes the handle safe: when a pool slot is freed and later reused,
// the slot's generation changes, so an old handle to that slot no longer resolves -- turning a
// use-after-free into a clean nullptr instead of silently aliasing a different resource. `Tag`
// makes handles to different resource kinds distinct types (a MeshHandle can't be passed where a
// TextureHandle is expected).
template<typename Tag>
struct Handle {
    uint32_t index = 0;
    uint32_t generation = 0;  // 0 == null

    constexpr bool valid() const { return generation != 0; }
    constexpr bool operator==(const Handle& o) const { return index == o.index && generation == o.generation; }
    constexpr bool operator!=(const Handle& o) const { return !(*this == o); }
};

// Owns a set of `T` in stable storage (addresses never move) and hands out generational handles to
// them. Insertion reuses freed slots via a free list; each (re)use bumps the slot's generation so
// handles to a previously-freed slot are detected as stale. This is the single-owner container the
// GPU resource registry and (later) the render graph build on: everyone else holds a handle, not a
// pointer or a shared_ptr.
template<typename T, typename Tag = T>
class HandlePool {
   public:
    using HandleType = Handle<Tag>;

    HandleType insert(T value) {
        uint32_t idx;
        if (!m_free.empty()) {
            idx = m_free.back();
            m_free.pop_back();
        } else {
            idx = static_cast<uint32_t>(m_slots.size());
            m_slots.emplace_back();
        }
        Slot& s = m_slots[idx];
        s.value = std::move(value);
        s.generation = m_next_generation;
        s.alive = true;
        // Advance the generation source, never landing on 0 (reserved for the null handle).
        if (++m_next_generation == 0) {
            m_next_generation = 1;
        }
        ++m_live_count;
        return HandleType{idx, s.generation};
    }

    // Returns nullptr for a null handle, an out-of-range index, a freed slot, or a slot whose
    // generation no longer matches (i.e. the handle is stale -- use-after-free caught here).
    T* get(HandleType h) {
        Slot* s = slotFor(h);
        return s ? &s->value : nullptr;
    }
    const T* get(HandleType h) const {
        const Slot* s = slotFor(h);
        return s ? &s->value : nullptr;
    }

    bool contains(HandleType h) const { return slotFor(h) != nullptr; }

    // Frees the slot (if the handle is live) so it can be reused with a fresh generation. Returns
    // false if the handle was already stale/null.
    bool erase(HandleType h) {
        Slot* s = slotFor(h);
        if (!s) {
            return false;
        }
        s->value = T{};  // release any owned resources now
        s->alive = false;
        m_free.push_back(h.index);
        --m_live_count;
        return true;
    }

    size_t size() const { return m_live_count; }

    // Visit every live entry as fn(HandleType, T&), skipping freed slots. Needed whenever a caller
    // has to find entries by a property of the value rather than by handle -- e.g. locating the
    // voices currently playing a buffer that is about to be deleted.
    //
    // Do not insert or erase from within the callback: the loop bound is the slot count at entry,
    // and erasing invalidates the iteration's assumptions. Collect handles, then act after.
    template<typename Fn>
    void forEachHandle(Fn&& fn) {
        for (uint32_t idx = 0; idx < static_cast<uint32_t>(m_slots.size()); ++idx) {
            Slot& s = m_slots[idx];
            if (s.alive) {
                fn(HandleType{idx, s.generation}, s.value);
            }
        }
    }

   private:
    struct Slot {
        T value{};
        uint32_t generation = 0;
        bool alive = false;
    };

    const Slot* slotFor(HandleType h) const {
        if (h.generation == 0 || h.index >= m_slots.size()) {
            return nullptr;
        }
        const Slot& s = m_slots[h.index];
        if (!s.alive || s.generation != h.generation) {
            return nullptr;
        }
        return &s;
    }
    Slot* slotFor(HandleType h) {
        return const_cast<Slot*>(static_cast<const HandlePool*>(this)->slotFor(h));
    }

    // std::deque keeps element addresses stable as the pool grows.
    std::deque<Slot> m_slots;
    std::vector<uint32_t> m_free;
    uint32_t m_next_generation = 1;  // never 0
    size_t m_live_count = 0;
};

}  // namespace ICE
