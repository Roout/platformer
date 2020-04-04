#include "PhysicWorld.h"
#include <algorithm>    // std::for_each
#include <unordered_map>

void PhysicWorld::Step(const float dt, size_t iterations) {
    const auto step = dt / iterations;
    for(size_t i = 0; i < iterations; i++) {
        this->Step(step);
    }
}

void PhysicWorld::Step(const float dt) {
    std::vector<std::pair<size_t, size_t>> kinematicColliders;
    std::vector<std::pair<size_t, size_t>> staticColliders;
    
    static constexpr size_t maxExpectedSize { 5 };
	kinematicColliders.reserve(maxExpectedSize);
	staticColliders.reserve(maxExpectedSize);

    size_t kBodyIndex { 0 };
    for( auto& [kBody, optCallback]: m_kinematicBodies ) {
		bool hasCollide { false };
        const auto FindCollisions = 
            [lhsBodyIndex = kBodyIndex, &lhs = kBody, &hasCollide](
                    const auto& bodies, 
                    auto& colliders
            ) {
            size_t rhsBodyIndex { 0 };
            for(const auto& [rhs, opt]: bodies) {
                if(&rhs != &lhs && rhs.CanInteract(&lhs) && rhs.Collide(&lhs)) {
                    colliders.emplace_back(lhsBodyIndex, rhsBodyIndex);
                    hasCollide = true;
                }
                rhsBodyIndex++;
            }
        };

        // handle X-movement
		kBody.MoveX(dt);
		// collisions with static entities
        FindCollisions(m_staticBodies, staticColliders);
        // collisions with kinematic entities
        FindCollisions(m_kinematicBodies, kinematicColliders);
        if (hasCollide) {
			kBody.RestoreX(); // restore position before collision
        }

		// handle Y-movement
        hasCollide = false;
		kBody.MoveY(dt);
		// collisions with static entities
		FindCollisions(m_staticBodies, staticColliders);
		// collisions with kinematic entities
        FindCollisions(m_kinematicBodies, kinematicColliders);
        if (hasCollide) {
            /* It can collide when:
                - is jumping up and colide => is going to fall down
                - is falling down and collide => is going to stand on the ground
            */
			kBody.RestoreY();
			if(!kBody.IsFallingDown()) {
                kBody.StartFall();
            }
		}
        // update body index
        kBodyIndex++;
    }

    // handle all callbacks,
	// e.g. visual effects, sound effects, state changing etc

    const auto InvokeCallback = [](const size_t id, const auto& container) {
        const auto& callback = container[id].second;
        if(callback) {
            std::invoke(*callback);
        }
    };

    std::unordered_map<size_t, size_t> invoked;
    invoked.reserve(kinematicColliders.size());
    for(const auto [lhs, rhs]: kinematicColliders) {
        // if we haven't invoke callbacks for these kinematic objects
        if( auto it = invoked.find(rhs); it == invoked.cend()) {
            InvokeCallback(lhs, m_kinematicBodies);
            InvokeCallback(rhs, m_kinematicBodies);
            // exclude reverse pair from calling callbacks
            invoked.insert(it, { rhs, lhs } );
        }
    }
    for(const auto [lhs,rhs]: staticColliders) {
        InvokeCallback(lhs, m_kinematicBodies);
        InvokeCallback(rhs, m_staticBodies);
    }
}
