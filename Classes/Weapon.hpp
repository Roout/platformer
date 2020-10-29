#ifndef WEAPON_SYSTEM_HPP
#define WEAPON_SYSTEM_HPP

#include "cocos2d.h"
#include "Utils.hpp"

#include <cinttypes>
#include <array>
#include <functional>

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

    Weapon(
        float damage, 
        float range, 
        float preparationTime, 
        float attackTime, 
        float reloadTime
    ):
        m_damage { damage },
        m_range { range }
    {
        m_durations[Utils::EnumCast(State::READY)] = 0.f; 
        m_durations[Utils::EnumCast(State::PREPARATION)] = preparationTime; 
        m_durations[Utils::EnumCast(State::ATTACK)] = attackTime; 
        m_durations[Utils::EnumCast(State::RELOAD)] = reloadTime; 
    }

    virtual ~Weapon() = default;

    /**
     * Update weapon state.
     */
    virtual void UpdateState(const float dt) noexcept {
        if( m_state != State::READY ) {
            m_timer -= dt;
            if( m_timer <= 0.f ) {
                this->NextState();
                if(m_state == State::ATTACK) {
                    this->OnAttack();
                }
            } 
        }
    }

    /**
     * @return 
     *      The indication whether the weapon can be used to attack or not
     */
    [[nodiscard]] bool IsReady() const noexcept {
        return m_state == State::READY;
    }

    [[nodiscard]] bool IsPreparing() const noexcept {
        return m_state == State::PREPARATION;
    }

    [[nodiscard]] bool IsAttacking() const noexcept {
        return m_state == State::ATTACK;
    }

    [[nodiscard]] bool IsReloading() const noexcept {
        return m_state == State::RELOAD;
    }

    /**
     * @param extractPosition A provided callback which extract the current
     * position from the player and generate an area where the projectile will be born. 
     * 
     * @param modifier The callable modificator object that push the body:
     * - setting velocity
     * - setting impulse
     * - setting force
     * etc
     */
    void LaunchAttack (
        std::function<cocos2d::Rect()>&& extractPosition,
        std::function<void(cocos2d::PhysicsBody*)>&& modifier
    ) noexcept {
        if(this->IsReady()) {
            // go to preparation state
            this->NextState();
            // save projectile properties
            m_extractor = std::move(extractPosition);
            m_modifier = std::move(modifier);
        }
    };

    /**
     * This function return the damage this weapon deal.
     * @return 
     *      Damage dealt by this weapon.  
     */
    [[nodiscard]] float GetDamage() const noexcept {
        return m_damage;
    }

    /**
     * This function return the attack range, i.e. 
     * the range in which this weapon can deal damage..
     * @return 
     *      Range dealt by this weapon.  
     */
    [[nodiscard]] float GetRange() const noexcept {
        return m_range;
    }

protected:

    virtual void OnAttack() = 0;
    
    // extract projectile size and spawn position from the current
    // state of the player.
    std::function<cocos2d::Rect()> m_extractor{};
    // projectile velocity: direction & speed
    std::function<void(cocos2d::PhysicsBody*)> m_modifier{};

private:
    enum class State : std::uint16_t {
        // weapon is ready to initiate an attack
        READY,
        // attack was initiated but is in preparation phase, 
        // e.g. load the arrow and pull the string or make a swing
        PREPARATION,
        // an actual attack, hopefully dealing a damage, 
        // i.e. create the projectile,
        // e.g. fire the arrow, bullet
        ATTACK,
        // additional reloading time
        RELOAD,
        // number of states
        COUNT
    };

    void NextState() noexcept {
        const auto next = (Utils::EnumCast(m_state) + 1) % Utils::EnumSize<State>();
        m_state = Utils::EnumCast<Weapon::State>(next);
        m_timer = m_durations[next];
    }

private:

    std::array<float, Utils::EnumSize<State>()> m_durations;

    State m_state { State::READY };

    float m_timer { 0.f };

    /**
     * Define a damage which the weapon deal on attack. 
     */
    const float m_damage { 0.f };
    
    /**
     * Define a weapon attack range (for melee it's swing range).
     */
    const float m_range { 20.f };
};

class Sword final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class Axe final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class Spear final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class Fireball final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class SlimeShot final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class Bow final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class Legs final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

#endif // WEAPON_SYSTEM_HPP