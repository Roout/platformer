#ifndef UNIT_HPP
#define UNIT_HPP

#include "Core.hpp"

class KinematicBody;
class PhysicWorld;

namespace cocos2d {
    class DrawNode; 
}

class Unit final : public core::Entity { 
public:
    enum class State {
        idle,
        melee_attack
    };
    
    Unit(PhysicWorld *world, float x, float y);

    ~Unit();

    [[nodiscard]] KinematicBody * GetBody() noexcept {
        return m_body;
    }

    [[nodiscard]] const KinematicBody * GetBody() const noexcept {
        return m_body;
    }

    /**
     * This function initiate a melee attack. 
     */
    void MeleeAttack() noexcept {
        // create rectangle/circle shape (!not physic body) using weapon attack range
        // detect collision and identify attack targets:
        //     go through all enemies around
        //         check for intersection with their shape
        //         if intersection occure
        //             add enemy to affected targets
        // foreach affected target
        //     unit->apply_weapon_affect(target) // deal damage
    }

private:
    // banch of just observer pointers.
    PhysicWorld * const     m_physicWorld { nullptr };
    KinematicBody * const   m_body { nullptr };

    State m_state { State::idle };

    static constexpr float m_width { 16.f };
    static constexpr float m_height { 28.f };
};

#endif // UNIT_HPP