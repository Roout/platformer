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

    /**
     * @param x defines the horizontal direction of movement by unit vector
     * @param y defines the vectical direction of movement by unit vector
     * Possible values are: { 1, 0 }, {-1, 0 }, { 0, 1 }, { 0, -1 }, { 0, 0 }, { 1, 1 }, {-1, -1 }
     * { 0, 0 } - means to stop movement 
     */
    void MoveAlong(float x, float y) noexcept override;
};

} // namespace Enemies
#endif // ENEMY_WASP_HPP