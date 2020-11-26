#ifndef ENEMIES_CANNON_HPP
#define ENEMIES_CANNON_HPP

#include "../Bot.hpp"

namespace Enemies {

class Cannon final : public Bot {
public:

    static Cannon* create(size_t id, const cocos2d::Size& contentSize, float scale);

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

    void onEnter() override;

/// Bot interface

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

    /**
     * Generate projectile at unit's center 
     */
    void Attack() override;

    void TryAttack() override;

private:
    enum WeaponClass{ RANGE };

    Cannon(size_t id, const cocos2d::Size& contentSize, float scale);

/// Bot interface
   
    void UpdateState(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;

    void AddWeapons() override;

    bool NeedAttack() const noexcept override;

private:

    float m_scale { 1.f }; 
};

} // namespace Enemies 

#endif  // UNITS_CANNON_HPP