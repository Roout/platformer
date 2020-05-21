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
static void InvokeCallback (
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


/**
 * This class update position of the kinematic bodies and handle the collision
 * if any has occured.
 * 
 * @note 
 *      This class is a friend of the PhysicWorld class.
 */
struct CollisionResolver final {
public:
    using ColliderIndexes = std::vector<std::pair<size_t, size_t>>;

    CollisionResolver(PhysicWorld * const world ) :
        m_world { world }
    {
    }

    /**
     * This method set up enviromentfor the collision resolver class.
     * It initializes pointer to array of collider indexes which will be 
     * filled when the work will be done.  
     * 
     * Need to be called before the @UpdatePosition method.
     */
    void SetEnviroment( 
        ColliderIndexes * const colliderIndexes 
    ) {
        m_colliderIndexes = colliderIndexes;
        m_hasEnviroment = true;
    }

    /**
     * @note 
     *      Expect @SetEnviroment method be called before.  
     */
    template<class RHS_Collider>
    bool UpdatePosition(
        KinematicBody * const kinematicBody,
        const size_t kinematicBodyIndex,
        void (KinematicBody::*Move)(float),
        void (KinematicBody::*Restore)(),
        const float dt
    ) {
        if( !m_hasEnviroment ) {
            throw std::logic_error("Forgot to set up enviroment for the Collision resolver instance.");
        }

        (kinematicBody->*Move)(dt);

        size_t rhsBodyIndex { 0 };
        bool wasAdjusted { false };
        auto lhs = kinematicBody;

        auto colliderBodies { this->GetColliderBodies<RHS_Collider>() }; 

        for(const auto& [rhs, opt]: *colliderBodies) {
            if(rhs != lhs && m_world->DetectCollision(*lhs, *rhs)) {
                m_colliderIndexes->emplace_back(kinematicBodyIndex, rhsBodyIndex);
                // move body to position which is closet to second collided body
                // if it wasn't moved before.
                if(!wasAdjusted) {
                    (lhs->*Restore)();
                    // move along y-axis step by step (mini-step)
                    const float miniDeltaTime { dt / STEPS_TO_RESOLVE_COLLISION };
                    for(int i = 0; i < STEPS_TO_RESOLVE_COLLISION; i++) {
                        (lhs->*Move)(miniDeltaTime);
                        if(lhs->Intersect(rhs)) {
                            (lhs->*Restore)();
                            break;
                        }
                    }
                    wasAdjusted = true; 
                }
            }
            rhsBodyIndex++;
        }

        m_hasEnviroment = false;
        return wasAdjusted;
    }

    /// hidden methods
private:

    /**
     * @template types
     *      RHS_Collider is a type of either KinematicBody or StaticBody.
     * 
     * @return
     *      This function return pointer to kinematic bodies container from the physic world 
     *      if template parameter is KinematicBody and StaticBody otherwise.  
     */
    template<class RHS_Collider>
    auto GetColliderBodies() const noexcept {
        if constexpr (std::is_same_v<RHS_Collider, KinematicBody>) {
            return &m_world->m_kinematicBodies;
        } else {
            return &m_world->m_staticBodies;
        }
    }

    /// data members
private:
    PhysicWorld * const m_world { nullptr };

    /**
     * @note 
     *      - it has delayed initialization.
     *      - can be changed!
     */
    ColliderIndexes * m_colliderIndexes { nullptr };

    /**
     * This flag indicate whether enviroment was set up before the 
     * @UpdatePosition call or not.
     * 
     * It forces to set up (even if it was same before) enviroment
     * each time adter the @UpdatePosition call.
     */
    bool m_hasEnviroment { false };

    /**
     * Constatant which describe how detailed the collision handled by 
     * the main algorithm (simple simulation of the movement until it collide or 
     * delta time consumed). The more this value the smaller steps will 
     * kinematic body do and more work will be done (so overhead too). 
     */
    static constexpr size_t STEPS_TO_RESOLVE_COLLISION { 5 };
};

void PhysicWorld::Step(const float dt) {
    // Pairs of [kinematic body index, kinematic body index] 
    std::vector<std::pair<size_t, size_t>> kinematicColliders;
    // Pairs of [kinematic body index, static body index] 
    std::vector<std::pair<size_t, size_t>> staticColliders;
    
    static constexpr size_t maxExpectedSize { 5 };
	kinematicColliders.reserve(maxExpectedSize);
	staticColliders.reserve(maxExpectedSize);

    CollisionResolver resolver { this };

    size_t kinematicBodyIndex { 0 };
    for( auto& [kinematicBody, optCallback]: m_kinematicBodies ) {
        /// handle X-movement
        // collisions with static entities
        resolver.SetEnviroment(&staticColliders);
        bool xAxisCollisionOccured = resolver.UpdatePosition<StaticBody>(
            kinematicBody, 
            kinematicBodyIndex, 
            &KinematicBody::MoveX, 
            &KinematicBody::RestoreX,
            dt 
        );
        
        // collisions with kinematic entities
        // TODO:  FindCollisions(m_kinematicBodies, kinematicColliders);

        // Update on ground state after moving along X-axis 
        // (unit may already leave ground) 

        // Indicate whether it's POSSIBLE for the body to fall down 
        // after moving along X-axis
        bool isFallingAfterXMove { false };
        // Expression (kBody->m_direction.y > 0.f) == true, means that this body 
        // is starting jumping
        if( kinematicBody->m_onGround && kinematicBody->m_direction.y == 0.f) {
            // assume that it was moved along x-axis and 
            // there wasn't anything to step on.
            kinematicBody->StartFall();
            // it's possible that the body will fall down!
            isFallingAfterXMove = true;
        }
		

		/// handle Y-movement
        // find y-axis collision with static bodies
        resolver.SetEnviroment(&staticColliders);
        bool yAxisCollisionOccured = resolver.UpdatePosition<StaticBody>(
            kinematicBody, 
            kinematicBodyIndex, 
            &KinematicBody::MoveY, 
            &KinematicBody::RestoreY,
            dt 
        );
        
        // if a collision occured
        if(yAxisCollisionOccured) {
            // it was falling down, but collide
            if( kinematicBody->IsFallingDown() ) {
                // so stop falling
                kinematicBody->m_direction.y = 0.f;
                kinematicBody->m_onGround = true;
            } else { // it was in jump state or on the ground
                kinematicBody->StartFall();
            }
        } else if( kinematicBody->m_jumpTime > 0.f ) {
            kinematicBody->m_onGround = false;
        } else if(isFallingAfterXMove) {
            // no collision occured so a body is falling down
            kinematicBody->m_onGround = false;
        }
        
        // the body was jumping and reached the highest point of the jump
        if( !yAxisCollisionOccured && kinematicBody->m_jumpTime <= 0.f && kinematicBody->m_direction.y > 0.f ) {
            kinematicBody->StartFall();
            kinematicBody->m_onGround = false;
        } 

		// collisions with kinematic entities
        // TODO:  FindCollisions(m_kinematicBodies, kinematicColliders);
       
        // update body index
        kinematicBodyIndex++;
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
