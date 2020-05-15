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
 *      from the PhysicWorld class. Used to access the entity passed as parameter to callback.
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
            [this, lhsBodyIndex = kBodyIndex, lhs = kBody, &hasCollide](
                    const auto& bodies, 
                    auto& colliders
            ) {
            size_t rhsBodyIndex { 0 };
            for(const auto& [rhs, opt]: bodies) {
                if(rhs != lhs && this->DetectCollision(*lhs, *rhs)) {
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

		/// handle Y-movement
        
        // indicate if whether it's possible for the body to fall down 
        // after moving along X-axis
        bool isFallingAfterXMove { false };
        // Expression (kBody->m_direction.y > 0.f) == true, means that this body 
        // is starting jumping
        if( kBody->m_onGround && kBody->m_direction.y <= 0.f) {
            // assume that it was moved along x-axis and 
            // there wasn't anything to step on.
            kBody->StartFall();
            // it's possible that the body will fall down!
            isFallingAfterXMove = true;
        }
		
        kBody->MoveY(dt);

        { // find y-axis collision with static bodies
            bool wasAdjusted { false };
            size_t rhsBodyIndex { 0 };
            for(const auto& [rhs, opt]: m_staticBodies) {
                // don't break the loop to mark all blocks the body collided with.
                if(this->DetectCollision<StaticBody>(*kBody, *rhs)) {
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
                            if(rhs->Intersect(kBody)) {
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
            if(wasAdjusted) {
                // it was falling down, but collide
                if( kBody->IsFallingDown() ) {
                    // so stop falling
                    kBody->m_direction.y = 0.f;
                    kBody->m_onGround = true;
                } else { // it was in jump state or on the ground
                    kBody->StartFall();
                }
            } else if( kBody->m_jumpTime > 0.f ) {
                kBody->m_onGround = false;
            } else if(isFallingAfterXMove) {
                // no collision occured so a body is falling down
                kBody->m_onGround = false;
            }
        }
		// collisions with kinematic entities
        /// TODO: implement this; for now it's ignoring collision with kinematic colliders
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
