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

    using PositionGenerator = std::function<cocos2d::Rect()>;
    using VelocityGenerator = std::function<void(cocos2d::PhysicsBody*)>;
    
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

    Weapon(const Weapon&) = default;
    Weapon& operator=(const Weapon&) = default;

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
     */
    void AddPositionGenerator(PositionGenerator extractPosition) noexcept {
        m_extractor = std::move(extractPosition);
    }

    /**
     * @param modifier The callable modificator that push the body of projectile:
     * - setting velocity
     * - setting impulse
     * - setting force
    */
    void AddVelocityGenerator(VelocityGenerator modifier) noexcept {
        m_modifier = std::move(modifier);
    }

    void LaunchAttack() noexcept {
        if (this->IsReady()) {
            // go to preparation state
            this->NextState();
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

    void ForceReload() noexcept {
        m_state = Weapon::State::RELOAD;
        m_timer = m_durations[Utils::EnumCast(m_state)];
    }

protected:

    virtual void OnAttack() = 0;
    
    // extract projectile size and spawn position from the current
    // state of the player.
    std::function<cocos2d::Rect()> m_extractor{};
    // projectile velocity: direction & speed
    std::function<void(cocos2d::PhysicsBody*)> m_modifier{};

protected:
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

protected:

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

class GenericAttack final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

using Axe = GenericAttack;

using Spear = GenericAttack;

using Maw = GenericAttack;

using Sting = GenericAttack;

class PlayerFireball final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class BossFireball final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;

    void UpdateState(const float dt) noexcept override;

private:

    static constexpr float DELAY { 0.3f };

    // Delay between fireball spawn 
    float m_delay { DELAY };

    bool m_attackedTwice { false };
};

class PlayerSpecial final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class SlimeShot final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class CloudFireball final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

namespace json_autogenerated_classes {
    struct UnitsFirecloud;
} // namespace json_autogenerated_classes
namespace json_models = json_autogenerated_classes;

class BossFireCloud final : public Weapon {
public:
    BossFireCloud(float damage, 
        float range, 
        float preparationTime, 
        float attackTime, 
        float reloadTime,
        const json_models::UnitsFirecloud *model
    );

    void OnAttack() override;

private:
    const json_models::UnitsFirecloud *m_model { nullptr };
};

using BossChainSweep = GenericAttack;

class BossChainSwing final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;

    void UpdateState(const float dt) noexcept override;

private:

    // delay was choosen base on animation duration
    static constexpr float DELAY { 0.3f };

    // Delay between fireball spawn 
    float m_delay { DELAY };

    bool m_attackedTwice { false };
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

class Stake final : public Weapon {
public:
    using Weapon::Weapon;

    void OnAttack() override;
};

class StalactitePart final : public Weapon {
public:
    StalactitePart(float damage
        , float range
        , float preparationTime 
        , float attackTime 
        , float reloadTime
        , size_t index
    );

    void OnAttack() override;
    
private:

    size_t m_index { 1 };
};

#endif // WEAPON_SYSTEM_HPP