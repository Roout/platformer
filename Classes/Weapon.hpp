#ifndef WEAPON_SYSTEM_HPP
#define WEAPON_SYSTEM_HPP

#include "Core.hpp"
#include "cocos2d.h"
#include <vector>
#include <memory>

class ProjectileView;

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
     * @param[in] position
     *      A left-bottom corner of the created projectile.
     * 
     * @param[in] velocity
     *      The velocity where the projectile will move.
     */
    virtual void Attack (
        const cocos2d::Vec2& position,
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

/**
 * This class represent the objects created by weapon. 
 * It used to track which targets did the weapon hit.
 * It has lifetime and body.
 * Melee and range weapons generate at least one projectile.   
 * 
 * @note
 *      It used ONLY to track collision, invoke callback, 
 *      manage the lifetime and disappear on first collision
 *      (whatever it callide) or when lifetime is over.
 */
class Projectile : public core::Entity {
public:
    // Lifecycle managment
    Projectile(const Projectile&) = delete;
    Projectile& operator=(const Projectile&) = delete;

    Projectile(Projectile&&) = default;
    Projectile& operator=(Projectile&&) = default;

public:

    Projectile(
        const cocos2d::Vec2& position,
        const cocos2d::Size& size,
        const cocos2d::Vec2& velocity,
        const float speed
    );

    ~Projectile();

    /**
     * This function update projectile state by keeping track 
     * of it's lifetime.
     */
    void Update(const float dt) noexcept {
        if (m_lifeTime > 0.f) {
            m_lifeTime -= dt;
        }
    }
   
    /**
     * This function tells whether this prjectile still exist or not.
     * @return 
     *      The indication that the projectile still exist. 
     */
    [[nodiscard]] bool IsExist() const noexcept {
        return m_lifeTime > 0.f;
    }

    [[nodiscard]] cocos2d::PhysicsBody * GetBody() noexcept {
        return m_body;
    }

    [[nodiscard]] const cocos2d::PhysicsBody * GetBody() const noexcept {
        return m_body;
    }

    /**
     * This compare operator overload usefull for sorting projectiles by lifeTime.
     * @return
     *      The indication that this projectile's life time is shorter 
     *      than the lifetime of the object you compare it too. 
     */
    [[nodiscard]] bool operator< (const Projectile& rhs) const noexcept {
        return m_lifeTime < rhs.m_lifeTime;
    }

    /**
     * This method ends the projectile lifetime. So it will disappear. 
     */
    void Collapse() noexcept {
        m_lifeTime = 0.f;
    }

    // Properties
private:
    
    /**
     * Keep track of projectile lifetime. When 'm_lifeTime' <= 0.f
     * projectile should disappear.
     */
    float m_lifeTime { 0.f };

    /**
     * Define an area where the attack can reach and do something, e.g. deal some damage. 
     * Exist until it collide with something or the projectile lifetime ends.
     */
    cocos2d::PhysicsBody * m_body { nullptr };

    /**
     * The view of the projectile.
     */
    ProjectileView * m_view { nullptr };
};


class Sword final : public Weapon {
public:
    using Weapon::Weapon;

    /**
     * This method create the sword's projectile using 
     * information from weapon owner and weapon. 
     * 
     * @param[in] position
     *      A left-bottom corner of the created projectile.
     * 
     * @param[in] direction
     *      The direction where the projectile will move.
     */
    void Attack(
        const cocos2d::Vec2& position,
        const cocos2d::Vec2& direction
    ) noexcept override;
    
// TODO: move all part from below to base class
    /**
     * Update reload time.
     * Update projectiles. 
     */
    void Update(const float dt) noexcept override;

    // Methods
private:

    /**
     * This method updates projectiles lifetime and erases expired.
     */
    void UpdateProjectiles(const float dt) noexcept;

    // Properties
private:

    /**
     * Keep and manage created but yet expired or collided projectiles.
     * Projectiles are sorted in decreasing order. 
     */
    std::vector<std::unique_ptr<Projectile>> m_projectiles;
};

#endif // WEAPON_SYSTEM_HPP