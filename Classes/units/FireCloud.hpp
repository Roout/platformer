#ifndef ENEMIES_FIRECLOUD_HPP
#define ENEMIES_FIRECLOUD_HPP

#include "../Bot.hpp"
#include "../Utils.hpp"

namespace Enemies {

class FireCloud final : public Bot {
public:

    static FireCloud* create(size_t id, const cocos2d::Size& contentSize);

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

    FireCloud(size_t id, const cocos2d::Size& contentSize);

/// Bot interface
   
    void UpdateState(const float dt) noexcept override;

    void UpdateAnimation() override;

    void UpdateWeapons(const float dt) noexcept override;

    void UpdatePosition(const float dt) noexcept override;

    void TryAttack() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;

    void AddWeapons() override;

    bool NeedAttack() const noexcept override;

private:

    static constexpr float MAX_SHELL_DELAY { 0.7f };

    static constexpr int MAX_SHELL_COUNT { 4 };

    static constexpr float CLOUD_LIFETIME { 4.f };
    
    static constexpr float CLOUD_SPEED { 100.f };

    bool m_finished { false };

    float m_lifetime { CLOUD_LIFETIME };

    float m_shellRenewTimer { 0.f };

    int m_shells { MAX_SHELL_COUNT };
};

} // namespace Enemies 

#endif // ENEMIES_FIRECLOUD_HPP 