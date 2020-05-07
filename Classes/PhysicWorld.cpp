#include "PhysicWorld.hpp"
#include "Core.hpp"
#include <algorithm>    // std::for_each
#include <unordered_map>

void PhysicWorld::Step(const float dt, size_t iterations) {
    const auto step = dt / iterations;
    for(size_t i = 0; i < iterations; i++) {
        this->Step(step);
    }
}


/**
 * This function invokes the callback of the collider with index 'id'
 * from the 'container'.
 * 
 * @param[in] callerIndex
 *      This is an index of the entity from the 'callerContainer' 
 *      which is one of the colliders. It provide the callback on creation.
 * 
 * @param[in] const reference to callerContainer
 *      This is a container that keeps track of the either kinematic either static bodies
 *      from the PhysicWorld class.
 * 
 * @param[in] argIndex
 *      This is an index of the entity from the 'argContainer' 
 *      which is another collider. It used as parametr for the callback.
 * 
 * @param[in] reference to callerContainer
 *      This is a container that keeps track of either kinematic either static bodies
 *      from the PhysicWorld class. Used to access the entity passed as parametr to callback.
 */
template <
    class Container1, 
    class Container2
>  
inline void InvokeCallback (
    const size_t callerIndex, 
    const Container1& callerContainer,
    const size_t argIndex, 
    Container2& argContainer
) noexcept {
    const auto& callback = callerContainer[callerIndex].second;
    auto & body = argContainer[argIndex].first;
    if( callback && body->HasModel() ) {
        body->InvokeCallback(*callback);
    }
};

void PhysicWorld::Step(const float dt) {
    std::vector<std::pair<size_t, size_t>> kinematicColliders;
    std::vector<std::pair<size_t, size_t>> staticColliders;
    
    static constexpr size_t maxExpectedSize { 5 };
	kinematicColliders.reserve(maxExpectedSize);
	staticColliders.reserve(maxExpectedSize);

    size_t kBodyIndex { 0 };
    for( auto& [kBody, optCallback]: m_kinematicBodies ) {
		bool hasCollide { false };
        /// TODO: refactore this to other free function
        const auto FindCollisions = 
            [lhsBodyIndex = kBodyIndex, lhs = kBody, &hasCollide](
                    const auto& bodies, 
                    auto& colliders
            ) {
            size_t rhsBodyIndex { 0 };
            for(const auto& [rhs, opt]: bodies) {
                if(rhs != lhs && rhs->CanInteract(lhs) && rhs->Collide(lhs)) {
                    colliders.emplace_back(lhsBodyIndex, rhsBodyIndex);
                    hasCollide = true;
                }
                rhsBodyIndex++;
            }
        };

        // handle X-movement
		kBody->MoveX(dt);
		// collisions with static entities
        FindCollisions(m_staticBodies, staticColliders);
        // collisions with kinematic entities
        FindCollisions(m_kinematicBodies, kinematicColliders);
        if (hasCollide) {
			kBody->RestoreX(); // restore position before collision
        }

		// handle Y-movement
		kBody->MoveY(dt);
        { // find y-axis collision with static bodies
            bool wasAdjusted { false };
            size_t rhsBodyIndex { 0 };
            for(const auto& [rhs, opt]: m_staticBodies) {
                // don't break the loop to mark all blocks the body collided with.
                if(rhs != kBody && rhs->CanInteract(kBody) && rhs->Collide(kBody)) {
                    staticColliders.emplace_back(kBodyIndex, rhsBodyIndex);
                    // move body to position which is closet to second collided body
                    // if it wasn't moved before.
                    if(!wasAdjusted) {
                        kBody->RestoreY();
                        // move along y-axis step by step (mini-step)
                        constexpr int steps { 5 };
                        static const float miniDeltaTime { dt / steps };
                        bool hasCollision { false };
                        for(int i = 0; i < steps && !hasCollision; i++) {
                            kBody->MoveY(miniDeltaTime);
                            if(rhs->Collide(kBody)) {
                                hasCollision = true;
                                wasAdjusted = true;
                                kBody->RestoreY();
                            }
                        }
                    }
                }
                rhsBodyIndex++;
            }
            // if a collision occured
            if( wasAdjusted && !kBody->IsFallingDown()) {
                kBody->StartFall();
            }
        }
		// collisions with kinematic entities
        hasCollide = false;
        FindCollisions(m_kinematicBodies, kinematicColliders);
        if (hasCollide) {
			kBody->RestoreY();
			if(!kBody->IsFallingDown()) {
                kBody->StartFall();
            }
		}
        // update body index
        kBodyIndex++;
    }

    // handle all callbacks,
	// e.g. visual effects, sound effects, state changing etc
    std::unordered_map<size_t, size_t> invoked;
    invoked.reserve(kinematicColliders.size());
    for(const auto [lhs, rhs]: kinematicColliders) {
        // if we haven't invoke callbacks for these kinematic objects
        if( auto it = invoked.find(rhs); it == invoked.cend()) {
            InvokeCallback(lhs, m_kinematicBodies, rhs, m_kinematicBodies);
            InvokeCallback(rhs, m_kinematicBodies, lhs, m_kinematicBodies);
            // exclude reversed pair from repeating callbacks again
            invoked.insert(it, { rhs, lhs } );
        }
    }
    for(const auto [lhs, rhs]: staticColliders) {
        InvokeCallback(lhs, m_kinematicBodies, rhs, m_staticBodies);
        InvokeCallback(rhs, m_staticBodies, lhs, m_kinematicBodies);
    }
}
