#ifndef BOULDER_PUSHER_HPP
#define BOULDER_PUSHER_HPP

#include "Bot.hpp"

namespace Enemies {

class BoulderPusher final : public Bot {
public:

    static BoulderPusher* create(size_t id);

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
    BoulderPusher(size_t id);

/// Bot interface
   
    void UpdateState(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;

    void AddWeapon() override;

    bool NeedAttack() const noexcept override;
};

}

#endif // BOULDER_PUSHER_HPP