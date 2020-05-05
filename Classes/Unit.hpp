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

    ~Unit();

    [[nodiscard]] KinematicBody * GetBody() noexcept {
        return &m_body;
    }

    [[nodiscard]] const KinematicBody * GetBody() const noexcept {
        return &m_body;
    }

    void RecieveDamage(int damage) noexcept override;

    /**
     * This function initiate a melee attack. 
     */
    void MeleeAttack() noexcept;

    void UpdateWeapon(const float dt) noexcept;
    
private:

    PhysicWorld * const m_world { nullptr };

    KinematicBody m_body;

    int m_health { 100 };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    State m_state { State::idle_right };

    static constexpr float m_width { 80.f };
    static constexpr float m_height { 146.f };
};

#endif // UNIT_HPP