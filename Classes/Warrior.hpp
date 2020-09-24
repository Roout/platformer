#ifndef WARRIOR_HPP
#define WARRIOR_HPP

#include <memory>

#include "Navigator.hpp"
#include "Bot.hpp"
#include "Path.hpp"

namespace Enemies {

class Warrior : public Bot {
public:

    static Warrior* create(size_t id);

    [[nodiscard]] bool init() override;
    
    void update(float dt) override;

/// Unique to warrior

    void AttachNavigator(Path&& path);
    
/// Bot interface

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

protected:
    Warrior(size_t id, const char* dragonBonesName);

    void AddWeapon() override;

private:

/// Unique to warrior

    void Pursue(cocos2d::Node * target) noexcept;

    void Patrol() noexcept;

/// Bot interface
   
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

#endif // WARRIOR_HPP