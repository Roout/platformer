#ifndef ENEMIES_SLIME_HPP
#define ENEMIES_SLIME_HPP

#include <memory>

#include "Navigator.hpp"
#include "Bot.hpp"
#include "Path.hpp"

namespace Enemies {

class Slime : public Bot {
public:
    static Slime* create(size_t id, const cocos2d::Size& contentSizee);

    bool init() override;

    void update(float dt) override;

/// Unique to warrior

    void AttachNavigator(Path&& path);
    
/// Bot interface

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

private:
    enum WeaponClass { RANGE };

    Slime(size_t id, const char * name, const cocos2d::Size& contentSizee);

    /// Unique to slime

    void Patrol() noexcept;

    /// Bot's interface

    void UpdateState(const float dt) noexcept override;

    void UpdatePosition(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;
    
    void AddPhysicsBody() override;

    void AddAnimator() override;

    bool NeedAttack() const noexcept override;
    
    void AddWeapons() override;

    void Attack() override;
    
/// Properties
private:
    std::unique_ptr<Navigator> m_navigator { nullptr };
};

} // namespace Enemies

#endif // ENEMIES_SLIME_HPP