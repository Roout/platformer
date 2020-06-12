#ifndef UNIT_HPP
#define UNIT_HPP

#include "Core.hpp"
#include "PhysicWorld.hpp"
#include "Weapon.hpp"
#include <memory>

class Unit final : public core::Entity { 
public:
    enum class State {
        idle,
        move,
        jump,
        attack
    };
    
    Unit(PhysicWorld * const world, float x, float y);

    ~Unit();

    [[nodiscard]] KinematicBody * GetBody() noexcept {
        return m_body;
    }

    [[nodiscard]] const KinematicBody * GetBody() const noexcept {
        return m_body;
    }

    [[nodiscard]] State GetState() const noexcept {
        return m_state;
    }

    void RecieveDamage(int damage) noexcept override;

    /**
     * This function initiate a melee attack. 
     */
    void MeleeAttack() noexcept;

    void UpdateWeapon(const float dt) noexcept;
    
    void UpdateState(const float dt) noexcept;

private:

    PhysicWorld * const m_world { nullptr };

    KinematicBody * m_body { nullptr };

    int m_health { 100 };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    State m_state { State::idle };

private:
    enum class Side { left, right };

    Side m_lookSide { Side::left };
    /**
     *  
     */
    static constexpr float m_maxAttackTime { 0.5f };
    float m_attackTime { m_maxAttackTime };

    static constexpr float m_width { 80.f };
    static constexpr float m_height { 146.f };
};

#endif // UNIT_HPP