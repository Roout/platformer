#ifndef UNIT_HPP
#define UNIT_HPP

#include "Core.hpp"
#include "PhysicWorld.hpp"
#include "Weapon.hpp"
#include <memory>

class Unit final : public core::Entity { 
public:
    enum class State {
        idle_left,
        idle_right,
        move_left,
        move_right,
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

    void RecieveDamage(int damage) noexcept override;

    /**
     * This function initiate a melee attack. 
     */
    void MeleeAttack() noexcept;

private:

    PhysicWorld * const m_world { nullptr };

    BodyPointer<KinematicBody> m_body { nullptr };

    int m_health { 100 };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    State m_state { State::idle_right };

    static constexpr float m_width { 16.f };
    static constexpr float m_height { 28.f };
};

#endif // UNIT_HPP