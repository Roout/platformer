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

    enum class Side { left, right };

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

    [[nodiscard]] Side GetSide() const noexcept {
        return m_lookSide;
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
    {}

    void Update(const float dt) noexcept;

    void Jump() noexcept;
    void MoveRight() noexcept;
    void MoveLeft() noexcept;
    void Stop() noexcept;
    void StopXAxisMove() noexcept;

private:
    Unit * const m_unit { nullptr };

    static constexpr float  m_desiredVelocity { 550.f };
    // static constexpr float  m_jumpHeight { 255.f };
    // static constexpr float  m_timeToJumpApex { 0.55 };

    static constexpr int    m_timeStepsToCompletion { 6 };

    struct Counter {
        int remainingJumpSteps { 0 };
        int remainingMoveLeft { 0 };
        int remainingMoveRight { 0 };

        void Clear() {
            remainingJumpSteps = remainingMoveLeft = remainingMoveRight = 0;
        }

    } m_counter;
};

#endif // UNIT_HPP