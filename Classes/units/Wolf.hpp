#ifndef ENEMIES_WOLF_HPP
#define ENEMIES_WOLF_HPP

#include "Warrior.hpp"

namespace Enemies {

class Wolf : public Warrior {
public:
    static Wolf* create(size_t id, const cocos2d::Size& contentSize);

    bool init() override;

private:
    enum WeaponClass { MELEE };

    Wolf(size_t id, const char * name, const cocos2d::Size& contentSize);

    void AddWeapons() override;

    void Attack() override;

    void OnEnemyIntrusion() override;

    void OnEnemyLeave() override;

    bool NeedAttack() const noexcept override;

private:
    static constexpr float PURSUE_SPEED = 200.f;
    static constexpr float PATROL_SPEED = 100.f;
};

} // namespace Enemies

#endif // ENEMIES_WOLF_HPP