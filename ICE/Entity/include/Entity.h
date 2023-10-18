//
// Created by Thomas Ibanez on 16.11.20.
//

#ifndef ICE_ENTITY_H
#define ICE_ENTITY_H

#include <cstdlib>
#include <unordered_map>
#include <typeindex>
#include <bitset>
#include <queue>

namespace ICE {
    using Entity = std::uint32_t;
    using Signature = std::bitset<32>;

    class EntityManager {
	public:
		EntityManager(): entityCount(0) {}

		Entity createEntity() {
			Entity e = entityCount+1;
			//Reuse free ids before using the next
			if(releasedEntities.size() > 0) {
				e = releasedEntities.front();
				releasedEntities.pop();
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

		void setSignature(Entity e, Signature s) {
			signatures[e] = s; 
		}

		Signature getSignature(Entity e) const {
			return signatures.at(e);
		}

	private:
		std::queue<Entity> releasedEntities{};
		std::unordered_map<Entity, Signature> signatures{};
		std::uint32_t entityCount;
	};
}

#endif //ICE_ENTITY_H
