#ifndef UNIT_HPP
#define UNIT_HPP

#include "Core.hpp"
#include "CurseHub.hpp"
#include "Weapon.hpp"
#include "SmoothFollower.hpp"
#include "UnitMovement.hpp"
#include <memory>
#include "cocos2d.h"

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
    void MeleeAttack();

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

    Unit(const cocos2d::Size& size);

private:

    void UpdateWeapon(const float dt) noexcept;
    
    void UpdatePosition(const float dt) noexcept;

    void UpdateState(const float dt) noexcept;

    void UpdateCurses(const float dt) noexcept;

    void UpdateAnimation();
    
    /// Assisting functions
    void CreateBody(const cocos2d::Size&);

    void FlipX(const Side);

    std::string CreateAnimationName(Act state);

private:
    friend class Movement;

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

    Curses::CurseHub m_curses;

    Movement m_movement { this };

    int m_health { 100 };
    
    State m_currentState {};
    State m_previousState {};

    bool m_hasContactWithGround { false };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    /**
     * The duration of attack animation
     */
    static constexpr float m_maxAttackTime { 0.5f };
    float m_attackTime { m_maxAttackTime };

};

class Player final : public Unit {
public:
    static Player* create(const cocos2d::Size&);

    [[nodiscard]] bool init() override;

    void update(float dt) override;

    void setPosition(const cocos2d::Vec2& position) override;

private:
    Player(const cocos2d::Size&);

    std::unique_ptr<SmoothFollower> m_follower { nullptr };
};

#endif // UNIT_HPP