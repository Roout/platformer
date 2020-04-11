#ifndef UNIT_HPP
#define UNIT_HPP

#include "Core.hpp"
#include "PhysicWorld.hpp"

class Unit final : public core::Entity { 
public:
    enum class State {
        idle,
        move,
        jump,
        melee_attack
    };
    
    Unit(PhysicWorld * const world, float x, float y);

    ~Unit() = default;

    [[nodiscard]] KinematicBody * GetBody() noexcept {
        return m_body.get();
    }

    [[nodiscard]] const KinematicBody * GetBody() const noexcept {
        return m_body.get();
    }

    /**
     * This function initiate a melee attack. 
     */
    void MeleeAttack() noexcept;

private:

    BodyPointer<KinematicBody> m_body;

    State m_state { State::idle };

    static constexpr float m_width { 16.f };
    static constexpr float m_height { 28.f };
};

#endif // UNIT_HPP