#ifndef STALACTITE_HPP
#define STALACTITE_HPP

#include "../Bot.hpp"

namespace Enemies {

/**
 * - just collapce when the hero passed
 * - consist of idle/die animations 
 */
class Stalactite final : public Bot {
public:
    static Stalactite* create(
        size_t id
        , const cocos2d::Size& contentSize
        , float scale
    );

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

private: 
    Stalactite(size_t id
        , const cocos2d::Size& contentSize
        , float scale
        , size_t index
    );

    /// Bot interface
    
    void TryAttack();

    bool NeedAttack() const noexcept;
   
    void UpdateState(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;

    void AddWeapons() override;

private:
    enum WeaponClass{ RANGE };

    float m_scale { 1.f }; 

    // define what type of stalactite it is
    size_t m_index { 0 }; 

    bool m_alreadyAttacked { false };
};

} 

#endif // STALACTITE_HPP