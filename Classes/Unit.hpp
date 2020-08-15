#ifndef UNIT_HPP
#define UNIT_HPP

#include "CurseHub.hpp"
#include "UnitMovement.hpp"
#include <memory>
#include "cocos2d.h"

class Weapon;

class Unit : public cocos2d::Node { 
public:
    static Unit* create(const cocos2d::Size& size);

    [[nodiscard]] bool init() override;

    void update(float dt) override;

    /**
     * Used by curses to lower health value. 
     */
    void RecieveDamage(int damage) noexcept;

    /**
     * This function initiate a melee attack. 
     */
    virtual void Attack();

    // Some esoteric attempts to check
    [[nodiscard]] bool IsOnGround() const noexcept;

    [[nodiscard]] Movement& GetMovement() noexcept {
        return m_movement;
    }

    [[nodiscard]] int GetHealth() const noexcept {
        return m_health;
    }
    /**
     * Invoked from event listener on contact between sensor attached to 
     * unit's physics body and ground (e.g. platform).
     * 
     * @param[in] hasContactWithGround
     *      Variable indicate whether unit's sensor is in contact with froun or not.
     */
    void HasContactWithGround(bool hasContactWithGround) noexcept {
        m_hasContactWithGround = hasContactWithGround;
    }

    template<Curses::CurseType type, class ...Args>
    void AddCurse(size_t id, Args&&... args) noexcept {
        m_curses.AddCurse<type>(id, std::forward<Args>(args)...);
    }

    void RemoveCurse(size_t id) noexcept {
        m_curses.RemoveCurse(id);
    }

    bool IsLookingLeft() const noexcept {
        return m_currentState.m_side == Side::left;
    }
    
    void Turn() noexcept {
        m_previousState.m_side = m_currentState.m_side;
        m_currentState.m_side = (m_currentState.m_side == Side::left? Side::right: Side::left);
    }
    /// Types
protected:
    enum class Act {
        idle,
        move,
        jump,
        attack,
        dead
    };

    enum class Side { 
        left, 
        right 
    };

    struct State final {
        /**
         * Indicate what the unit is doing now 
         */
        Act m_act { Act::idle };
        /**
         * Indicate where the unit is looking now
         */
        Side m_side { Side::left };
    };

    friend class Movement;

protected:

    Unit(
        const cocos2d::Size& size, 
        const std::string& dragonBonesName
    );

    /// Update functions
    void UpdateWeapon(const float dt) noexcept;
    
    virtual void UpdatePosition(const float dt) noexcept;

    virtual void UpdateState(const float dt) noexcept;

    virtual void UpdateAnimation();

    void UpdateDebugLabel();

    void UpdateCurses(const float dt) noexcept;
    
    /// Assisting functions
    virtual void OnDeath();

    void CreateBody(const cocos2d::Size&);

    void FlipX(const Side);

    std::string CreateAnimationName(Act state);

    /// Properties
protected:
    Curses::CurseHub m_curses { this };

    int m_health { 100 };

    Movement m_movement;
    
    State m_currentState {};

    State m_previousState {};

    bool m_hasContactWithGround { false };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    const std::string m_dragonBonesName {};
    /**
     * The duration of attack animation
     */
    float m_maxAttackTime { 0.5f };
    
    float m_attackTime { m_maxAttackTime };
};

#endif // UNIT_HPP