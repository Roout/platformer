#ifndef BANDIT_BOSS_HPP
#define BANDIT_BOSS_HPP

#include <memory>

#include "Bot.hpp"

#include "../Navigator.hpp"
#include "../Path.hpp"

class Dash;

namespace Enemies {

class BanditBoss : public Bot {
public:

    // Defines how high can the body jump
    // Note, in formula: G = -H / (2*t*t), G and t are already defined base on player
    // so changing This JUMP_HEIGHT will just tweak 
    static constexpr float JUMP_HEIGHT { 80.f };

    static constexpr float DASH_SPEED { 600.f };

    static constexpr int MAX_HEALTH { 500 };

public:

    static BanditBoss* create(size_t id, const cocos2d::Size& contentSize);

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

/// Unique to boss

    void AttachNavigator(Path&& path);
    
/// Bot interface

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

protected:

    enum WeaponClass { 
        // Generate 2 fireballs while attacking by chains
        FIREBALL_ATTACK,    // animation name: attack_1
        // Dummy weapon used to track down timings and cooldowns of this type of attack.
        // Instead of creating projectiles it only generates a FireCloud
        FIRECLOUD_ATTACK,   // animation name: attack_2 
        // Jump dealing damage by sweeping chain attack below the boss
        SWEEP_ATTACK,       // animation name: attack_3
        DASH
    };

    BanditBoss(size_t id, const char* dragonBonesName, const cocos2d::Size& contentSize);

    void AddWeapons() override;

private:

/// unique to boss

    void LaunchFireballs();

    void LaunchFirecloud();

    void LaunchSweepAttack();

    void LaunchDash();

    /**
     * Check whether FIREBALL_ATTACK can be launched.
     * Consider:
     * 1. No cooldown
     * 2. Player exist and is alive
     * 3. No other attacks performed
     */
    bool CanLaunchFireballs() const noexcept;

    /**
     * Check whether FIRECLOUD_ATTACK can be launched.
     * Consider:
     * 1. No cooldown
     * 2. Player exist and is alive
     * 3. health is <= 50% 
     * 4. No other attacks performed
     */
    bool CanLaunchFirecloud() const noexcept;
    
    /**
     * Check whether SWEEP_ATTACK can be launched.
     * Consider:
     * 1. No cooldown
     * 2. Player exist and is alive
     * 3. Player is in jump range
     * 4. No other attacks performed
     */
    bool CanLaunchSweepAttack() const noexcept;

    /**
     * Check whether DASH can be launched.
     * Consider:
     * 1. No cooldown
     * 2. Player exist and is alive
     * 3. health <= 50%? only after [finishing fire cloud call]
     * 4. health > 50%? player is quite far from the boss
     * 5. No other attacks performed
     */
    bool CanLaunchDash() const noexcept;

/// Bot interface

    void TryAttack() override;
   
    void UpdateState(const float dt) noexcept override;

    void UpdatePosition(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;

/// Properties
private:
    std::unique_ptr<Navigator> m_navigator { nullptr };
    Dash *m_dash { nullptr };
};

} // namespace Enemies

#endif // BANDIT_BOSS_HPP