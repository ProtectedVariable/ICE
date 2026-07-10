//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_ENTITY_H
#define ICE_ENTITY_H

#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <typeindex>
#include <unordered_map>

namespace ICE {
using Entity = std::uint32_t;
// 64 bits allows up to 64 component types (8 built-in + custom). See registerComponent's
// bound check.
using Signature = std::bitset<64>;

// Reserved "no entity" / scene-graph-root sentinel.
constexpr Entity NULL_ENTITY = 0;

class EntityManager {
   public:
    EntityManager() : entityCount(0) {}

    Entity createEntity(Entity e = 0) {
        if (e == 0) {
            e = m_next_id;
            //Reuse free ids before using the next
            if (releasedEntities.size() > 0) {
                e = releasedEntities.front();
                releasedEntities.pop();
            }
        }
        if (e >= m_next_id) {
            m_next_id = e + 1;
        }
        entityCount++;

        signatures.insert_or_assign(e, Signature(0));
        return e;
    }

    void releaseEntity(Entity e) {
        // Guard against releasing a non-alive/already-released entity: that used to enqueue
        // the same id twice (later handed out as two live entities) and underflow the count.
        // Presence of a signature entry is the aliveness proxy.
        if (!signatures.contains(e)) {
            return;
        }
        signatures.erase(e);  // erase, not reset: the map was growing monotonically
        releasedEntities.push(e);
        if (entityCount > 0) {
            entityCount--;
        }
    }

    bool isAlive(Entity e) const { return e != NULL_ENTITY && signatures.contains(e); }

    void setSignature(Entity e, Signature s) { signatures[e] = s; }

    Signature getSignature(Entity e) const {
        if (e == 0 || !signatures.contains(e)) {
            return 0;
        }
        return signatures.at(e);
    }

   private:
    std::queue<Entity> releasedEntities{};
    std::unordered_map<Entity, Signature> signatures{};
    std::uint32_t entityCount;
    std::uint32_t m_next_id = 1;
};
}  // namespace ICE

#endif  //ICE_ENTITY_H
