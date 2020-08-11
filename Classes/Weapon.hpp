#ifndef WEAPON_SYSTEM_HPP
#define WEAPON_SYSTEM_HPP

#include "cocos2d.h"

/**
 * Weapon defines attack damage, attack range and attack speed
 * that unit may have. 
 */
class Weapon { 
public:
    // Lifecycle management 
    
    Weapon(const Weapon&) = default;
    Weapon& operator=(const Weapon&) = default;

public:
    /**
     * This constructor define the weapin base on provided 
     * damage, range and reload time values. 
     */
    Weapon(int damage, int range, float reloadTime):
        m_damage { damage },
        m_range { range },
        m_maxReloadTime { reloadTime }
    {
    }

    /**
     * Trivial virtual destructor. 
     */
    virtual ~Weapon() = default;

    /**
     * Update weapon state. Used for updating reload time. 
     */
    virtual void Update(const float dt) noexcept {
        if( m_reload > 0.f ) {
            m_reload -= dt;
        }
    }

    /**
     * This function knows this weapon's reload time and tells whether it's 
     * already can be used to attack or not.
     * 
     * @return 
     *      The indication whether the weapon can be used to attack or not,
     */
    [[nodiscard]] bool CanAttack() const noexcept {
        return m_reload <= 0.f;
    }

    /**
     * Attack by the weapon. 
     * - MUST force weapon to reload. 
     * - Doesn't attack anyone, but create the projectile.
     * 
     * @param[in] area
     *      An area which will be attacked by the weapon.
     * 
     * @param[in] velocity
     *      The velocity where the projectile will move.
     */
    virtual void Attack (
        const cocos2d::Rect& area,
        const cocos2d::Vec2& velocity
    ) noexcept = 0;

    /**
     * Force weapon to reaload, i.e. it's a cooldown.
     * Implemented to avoid non-stop attacks.  
     */
    void ForceReload() noexcept {
        m_reload = m_maxReloadTime;
    }

    /**
     * This function return the damage this weapon deal.
     * @return 
     *      Damage dealt by this weapon.  
     */
    [[nodiscard]] int GetDamage() const noexcept {
        return m_damage;
    }

    /**
     * This function return the attack range, i.e. 
     * the range in which this weapon can deal damage..
     * @return 
     *      Range dealt by this weapon.  
     */
    [[nodiscard]] int GetRange() const noexcept {
        return m_range;
    }

private:
    /**
     * Define a damage which the weapon deal on attack. 
     */
    const int m_damage { 0 };
    
    /**
     * Define a weapon attack range (for melee it's swing range).
     */
    const int m_range { 20 };

    /**
     * Define a weapon reload time, i.e. how much time must elapse 
     * from the last attack before we can attack again or what interval
     * is between the attacks.
     */
    const float m_maxReloadTime { 0.1f };

    /**
     * Define the current reload time of the weapon.
     * @note
     * You can attack when 'm_reload' <= 0.f otherwise you can't.
     */
    float m_reload { 0.f };
};

class Sword final : public Weapon {
public:
    using Weapon::Weapon;

    /**
     * This method create the sword's projectile using 
     * information from weapon owner and weapon. 
     * 
     * @param[in] area
     *      An area which will be attacked by the weapon.
     * 
     * @param[in] velocity
     *      The velocity where the projectile will move.
     */
    void Attack (
        const cocos2d::Rect& area,
        const cocos2d::Vec2& velocity
    ) noexcept override;
    
};

class Axe final : public Weapon {
public:
    using Weapon::Weapon;

    /**
     * This method create the axe's projectile using 
     * information from weapon owner and weapon. 
     * 
    * @param[in] area
     *      An area which will be attacked by the weapon
     *      and recieve damage
     * 
     * @param[in] velocity
     *      The velocity where the projectile will move.
     */
    void Attack (
        const cocos2d::Rect& area,
        const cocos2d::Vec2& velocity
    ) noexcept override;
};

#endif // WEAPON_SYSTEM_HPP