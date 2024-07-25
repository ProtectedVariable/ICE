//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_ENTITY_H
#define ICE_ENTITY_H

#include <bitset>
#include <cstdlib>
#include <queue>
#include <typeindex>
#include <unordered_map>

namespace ICE {
using Entity = std::uint32_t;
using Signature = std::bitset<32>;

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
        signatures[e].reset();
        releasedEntities.push(e);
        entityCount--;
    }

    void setSignature(Entity e, Signature s) { signatures[e] = s; }

    Signature getSignature(Entity e) const {
        if (e == 0) {
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
