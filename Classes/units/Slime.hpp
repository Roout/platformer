#ifndef ENEMIES_SLIME_HPP
#define ENEMIES_SLIME_HPP

#include <memory>

#include "Bot.hpp"

#include "../Navigator.hpp"
#include "../Path.hpp"

namespace json_autogenerated_classes {
    struct Slime;
} // namespace json_autogenerated_classes
namespace json_models = json_autogenerated_classes;

namespace Enemies {

class Slime : public Bot {
public:
    static Slime* create(size_t id
        , const cocos2d::Size& contentSize
        , const json_models::Slime *slime);

    bool init() override;

    void update(float dt) override;

/// Unique to warrior

    void AttachNavigator(Path&& path);
    
/// Bot interface

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

private:
    enum WeaponClass { RANGE };

    Slime(size_t id
        , const cocos2d::Size& contentSize
        , const json_models::Slime *slime);

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

/// Configs (Json Model Data):
    const json_models::Slime * const m_slime { nullptr };
};

} // namespace Enemies

#endif // ENEMIES_SLIME_HPP