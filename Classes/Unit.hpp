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

    [[nodiscard]] int GetHealth() const noexcept {
        return m_health;
    }
    
    // Some esoteric attempts to check
    [[nodiscard]] bool IsOnGround() const noexcept;

    /**
     * It's used by unit's view to update data about unit's state.
     * Update @m_isOnGround variable. 
     * 
     * Invoked from event listener on contact between sensor attached to 
     * unit's physics body and ground (e.g. platform).
     * 
     * @param[in] hasContactWithGround
     *      Variable indicate whether unit's sensor is in contact with froun or not.
     */
    void HasContactWithGround(bool hasContactWithGround) noexcept {
        m_hasContactWithGround = hasContactWithGround;
    }

private:
    cocos2d::PhysicsBody * m_body { nullptr };

    int m_health { 100 };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    State m_state { State::idle };  

    bool m_hasContactWithGround { false };

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