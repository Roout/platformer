#ifndef ENEMY_WASP_HPP
#define ENEMY_WASP_HPP

#include "Warrior.hpp"

namespace Enemies {

class Wasp : public Warrior {
public:
    static Wasp* create(size_t id, const cocos2d::Size& contentSize);

    bool init() override;

    void AttachNavigator(Path&& path) override;

private:
    enum WeaponClass { MELEE };

    Wasp(size_t id, const char * name, const cocos2d::Size& contentSize);

    void AddWeapons() override;

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

    bool NeedAttack() const noexcept override;

    void Attack() override;

    void Pursue(Unit * target) noexcept override;

    void AddPhysicsBody() override;

    void MoveAlong(Movement::Direction) noexcept override;
};

} // namespace Enemies
#endif // ENEMY_WASP_HPP