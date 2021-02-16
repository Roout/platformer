#ifndef BANDIT_BOSS_HPP
#define BANDIT_BOSS_HPP

#include <memory>

#include "../Navigator.hpp"
#include "../Bot.hpp"
#include "../Path.hpp"

namespace Enemies {

class BanditBoss : public Bot {
public:

    // Defines how high can the body jump
    // Note, in formula: G = -H / (2*t*t), G and t are already defined base on player
    // so changing This JUMP_HEIGHT will just tweak 
    static constexpr float JUMP_HEIGHT { 130.f };

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
        ATTACK_1,
        // Dummy weapon used to track down timings and cooldowns of this type of attack.
        // Instead of creating projectiles it only generates a FireCloud
        ATTACK_2, 
        // Jump dealing damage by sweeping chain attack below the boss
        ATTACK_3
    };

    BanditBoss(size_t id, const char* dragonBonesName, const cocos2d::Size& contentSize);

    void AddWeapons() override;

private:

/// unique to boss

    void Attack1();

    void Attack2();

    void Attack3();

    /**
     * Check whether Attack3 can be launched.
     * Consider:
     * 1. No cooldown
     * 2. Player exist and is alive
     * 3. Player is in jump range
     * 4. No other attacks performed
     */
    bool CanLaunchAttack3() const noexcept;

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
};

} // namespace Enemies

#endif // BANDIT_BOSS_HPP