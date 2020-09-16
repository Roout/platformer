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

    void pause() override;

    void resume() override;

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

    /// Movement interface

    void Stop() noexcept;

    void MoveLeft() noexcept;

    void MoveRight() noexcept;

    void Jump() noexcept;

    void Turn() noexcept;

protected:

    Unit(const std::string& dragonBonesName);

    /// Update functions

    virtual void UpdateState(const float dt) noexcept = 0;
    
    /**
     * Update animation according to the current state of Unit  
     */
    virtual void UpdateAnimation() = 0;

    virtual void UpdateWeapon(const float dt) noexcept;

    virtual void UpdatePosition(const float dt) noexcept;

    virtual void UpdateCurses(const float dt) noexcept;

    /**
     * Update a debug label above the unit head.
     * Usually used to show unit's current state.
     */
    virtual void UpdateDebugLabel() noexcept {};
    
    virtual void OnDeath() = 0;

    /**
     * Create and add physics body as component to the node.
     * Inheritor need to provide collision&contact masks.
     */
    virtual void AddPhysicsBody();

    /**
     * Create and prepare a dragon bones animator. 
     * Inheritor need to initialize it with required animations.
     * 
     * @note call this method at the begining of the overriden one
     */
    virtual void AddAnimator();

    /**
     * Create a weapon with desired parameters 
     */
    virtual void AddWeapon() = 0;

    /// Properties
protected:
    enum class Side { 
        LEFT, 
        RIGHT 
    };

    Side m_side { Side::LEFT };

    int m_health { 100 };

    Curses::CurseHub m_curses { this };

    std::unique_ptr<Movement> m_movement { nullptr };
    
    std::unique_ptr<Weapon> m_weapon { nullptr };

    // retain when add as child
    dragonBones::Animator *m_animator { nullptr };

    const std::string m_dragonBonesName {};

    cocos2d::Size m_designedSize {};
    
    bool m_hasContactWithGround { false };
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
    return m_side == Side::LEFT;
}

inline bool Unit::IsDead() const noexcept {
    return m_health <= 0;
}


#endif // UNIT_HPP