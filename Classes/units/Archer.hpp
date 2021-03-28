#ifndef ENEMIES_ARCHER_HPP
#define ENEMIES_ARCHER_HPP

#include "Bot.hpp"

namespace Enemies {

class Archer final : public Bot {
public:

    static Archer* create(size_t id, const cocos2d::Size& contentSize);

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

/// Bot interface

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

    /**
     * Generate projectile at unit's center 
     */
    void Attack() override;

private:
    enum WeaponClass{ RANGE };

    Archer(size_t id, const cocos2d::Size& contentSize);

/// Bot interface
   
    void UpdateState(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;

    void AddWeapons() override;

    bool NeedAttack() const noexcept override;
};

} // namespace Enemies 

#endif // ENEMIES_ARCHER_HPP 