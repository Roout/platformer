#ifndef UNIT_HPP
#define UNIT_HPP

#include "Core.hpp"
#include "Weapon.hpp"
#include <memory>
#include "cocos2d.h"

class Movement;

class Unit final : public core::Entity { 
public:
    enum class State {
        idle,
        move,
        jump,
        attack
    };
    
    Unit();

    ~Unit();

    void AddBody(cocos2d::PhysicsBody * const body) noexcept;

    [[nodiscard]] const cocos2d::PhysicsBody * GetBody() const noexcept {
        return m_body;
    }

    [[nodiscard]] cocos2d::Size GetSize() const noexcept;


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

    int GetHealth() const noexcept {
        return m_health;
    }
    
private:
    cocos2d::PhysicsBody * m_body { nullptr };

    int m_health { 100 };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    State m_state { State::idle };

private:
    enum class Side { left, right };

    Side m_lookSide { Side::left };
    /**
     * The duration of attack animation
     */
    static constexpr float m_maxAttackTime { 0.5f };
    float m_attackTime { m_maxAttackTime };

    static constexpr float  m_width { 80.f };
    static constexpr float  m_height { 135.f };    

    friend class Movement;
};

class Movement final {
public:
    Movement ( Unit * const unit ):
        m_unit { unit } 
    {
    }

    void MoveLeft() noexcept;
    void MoveRight() noexcept;
    void Jump() noexcept;
    void Stop() noexcept;
    void StopXAxisMove() noexcept;

private:
    Unit * const m_unit { nullptr };
};

#endif // UNIT_HPP