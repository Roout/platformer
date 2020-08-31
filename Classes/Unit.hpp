#ifndef UNIT_HPP
#define UNIT_HPP

#include "CurseHub.hpp"
#include "UnitMovement.hpp"
#include <memory>
#include "cocos2d.h"

namespace dragonBones {
    class Animator;
}
class Weapon;

class Unit : public cocos2d::Node { 
public:

    [[nodiscard]] bool init() override;

    /**
     * Used by curses to lower health value. 
     */
    void RecieveDamage(int damage) noexcept;

    /**
     * This function initiate an attack. 
     */
    virtual void Attack();

    // Some esoteric attempts to check
    [[nodiscard]] bool IsOnGround() const noexcept;

    [[nodiscard]] inline Movement& GetMovement() noexcept;

    [[nodiscard]] inline int GetHealth() const noexcept;

    /**
     * Invoked from event listener on contact between sensor attached to 
     * unit's physics body and ground (e.g. platform).
     * 
     * @param[in] hasContactWithGround
     *      Variable indicate whether unit's sensor is in contact with froun or not.
     */
    inline void SetContactWithGround(bool hasContactWithGround) noexcept;

    template<Curses::CurseClass type, class ...Args>
    void AddCurse(size_t id, Args&&... args) noexcept;

    inline void RemoveCurse(size_t id) noexcept;

    inline bool IsLookingLeft() const noexcept;
    
    inline bool IsDead() const noexcept;

    void Stop() noexcept;

    void MoveLeft() noexcept;

    void MoveRight() noexcept;

    void Jump() noexcept;

    void Turn() noexcept;

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

    Unit(const std::string& dragonBonesName);

    /// Update functions

    virtual void UpdateState(const float dt) noexcept = 0;
    
    virtual void UpdateAnimation() = 0;

    virtual void UpdateWeapon(const float dt) noexcept;

    virtual void UpdatePosition(const float dt) noexcept;

    virtual void UpdateCurses(const float dt) noexcept;

    virtual void UpdateDebugLabel();
    
    /// Assisting methods

    virtual void AddPhysicsBody(const cocos2d::Size&);

    virtual void AddAnimator() = 0;

    void FlipX();

    std::string CreateAnimationName(Act state);

    /// Properties
protected:
    // retain when add as child
    dragonBones::Animator *m_animator { nullptr };

    Curses::CurseHub m_curses { this };

    int m_health { 100 };

    Movement m_movement { this };
    
    State m_currentState {};

    State m_previousState {};

    bool m_hasContactWithGround { false };

    std::unique_ptr<Weapon> m_weapon { nullptr };

    const std::string m_dragonBonesName {};

    cocos2d::Size m_designedSize {};
};

/// Implementation

inline int Unit::GetHealth() const noexcept {
    return m_health;
}

inline void Unit::SetContactWithGround(bool hasContactWithGround) noexcept {
    m_hasContactWithGround = hasContactWithGround;
}

template<Curses::CurseClass type, class ...Args>
void Unit::AddCurse(size_t id, Args&&... args) noexcept {
    m_curses.AddCurse<type>(id, std::forward<Args>(args)...);
}

inline void Unit::RemoveCurse(size_t id) noexcept {
    m_curses.RemoveCurse(id);
}

inline bool Unit::IsLookingLeft() const noexcept {
    return m_currentState.m_side == Side::left;
}

inline bool Unit::IsDead() const noexcept {
    return m_currentState.m_act == Act::dead;
}


#endif // UNIT_HPP