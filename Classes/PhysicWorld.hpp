#ifndef OWN_PHYSIC_WORLD_HPP
#define OWN_PHYSIC_WORLD_HPP

#include <functional>   // std::function used for callback type
#include <vector>
#include <utility>      // std::pair 
#include <optional> 
#include <algorithm>    // std::find_if
#include <utility>      // std::swap
#include <cassert>
#include <type_traits>  // std::is_same_v, std::enable_if
#include <cmath>        // std::fabs

#include "PhysicBody.hpp"

namespace core {
    /**
     * Forward declaration of the base class of all entities in this game.
     * Provide a set of the methods which are used on collision, e.g. 
     * recieving the damage.
     */
    class Entity;
}

class PhysicWorld final {
public:
    /**
     * Callback function type. 
     * Define what the callback owner will do with entity (passed to callback as parametr)
     * when the collision occur.
     * 
     * @note
     *      For reference, the Box2d has onContactBegin, onContactEnd, etc. 
     */
    using OnCollision = std::function<void(core::Entity *)>;
    
    PhysicWorld() {
        m_staticBodies.reserve(10000);
        m_kinematicBodies.reserve(50);
    }

    void Step(const float dt, size_t iterations);

    /**
     * This method adds body to a physic world to be managed here.
     */
    template<class BodyType> 
    void Add(
        BodyType * const body, 
        std::optional<OnCollision> callback = std::nullopt
    );

    /**
     * This method erases body from the physic world.
     * 
     * @note
     *      It performs a linear search, a swap and a pop_back on vector. This leads to 
     *      possible reallocations etc. 
     */
    template<class BodyType> 
    void Erase(const BodyType* const body) noexcept;

private:
    void Step(const float dt);

    template<class BodyType>
    using MyPair = std::pair<BodyType*, std::optional<OnCollision>>;

    std::vector<MyPair<StaticBody>>     m_staticBodies;
    std::vector<MyPair<KinematicBody>>  m_kinematicBodies;
};

/////////////////////////////
// TEMPLATE IMPLEMENTATION //
/////////////////////////////

template<class BodyType> 
void PhysicWorld::Add(
    BodyType * const body,
    std::optional<OnCollision> callback
) {
    static_assert(
            std::is_same_v<BodyType, StaticBody> || 
            std::is_same_v<BodyType, KinematicBody>,
            "wrong body type" 
    );

    if constexpr (std::is_same_v<BodyType, StaticBody>) {
        m_staticBodies.emplace_back(body, callback);
    }
    else {
        m_kinematicBodies.emplace_back(body, callback);
    }
}

template<class BodyType> 
void PhysicWorld::Erase(const BodyType* const body ) noexcept {
    static_assert(
            std::is_same_v<BodyType, StaticBody> || 
            std::is_same_v<BodyType, KinematicBody>,
            "wrong body type" 
    );

    auto Remove = [body](std::vector<MyPair<BodyType>>& container) {
        auto it = std::find_if(
            container.begin(), 
            container.end(), 
            [body](const MyPair<BodyType>& cPair) {
                return (body == cPair.first);
            }
        );
        assert(it != container.end());
        // Order doesn't matter so I can just swap and pop element
        const auto backIt = --container.cend();
        // discard element pointed by iterator 'it'
        *it = *backIt;
        container.pop_back();
    };

    if constexpr (std::is_same_v<BodyType, StaticBody>) {
        Remove(m_staticBodies);
    }
    else {
        Remove(m_kinematicBodies);
    }
}

#endif // OWN_PHYSIC_WORLD_HPP