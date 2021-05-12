#ifndef WARRIOR_HPP
#define WARRIOR_HPP

#include <memory>

#include "../Navigator.hpp"
#include "../Path.hpp"

#include "Bot.hpp"

namespace Enemies {

class Warrior : public Bot {
public:

    static Warrior* create(size_t id, const cocos2d::Size& contentSize);

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

/// Unique to warrior

    virtual void AttachNavigator(Path&& path);
    
/// Bot interface

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

protected:

    enum WeaponClass { MELEE };

    Warrior(size_t id, const char* dragonBonesName, const cocos2d::Size& contentSize);

    void AddWeapons() override;

/// Unique to warrior

    virtual void Pursue(Unit * target) noexcept;

    virtual void Patrol() noexcept;
    
    void AddPhysicsBody() override;

private:
/// Bot interface
   
    void UpdateState(const float dt) noexcept override;

    void UpdatePosition(const float dt) noexcept override;

    void UpdateAnimation() override;

    void OnDeath() override;

    void AddAnimator() override;

/// Properties
protected:
    std::unique_ptr<Navigator> m_navigator { nullptr };
};

} // namespace Enemies

#endif // WARRIOR_HPP