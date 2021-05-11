#ifndef ENEMY_WASP_HPP
#define ENEMY_WASP_HPP

#include "Warrior.hpp"

namespace Enemies {

class Wasp : public Warrior {
public:
    static Wasp* Wasp::create(size_t id, const cocos2d::Size& contentSize);

    bool init() override;

private:
    enum WeaponClass { MELEE };

    Wasp(size_t id, const char * name, const cocos2d::Size& contentSize);

    void AddWeapons() override;

    void Attack() override;
};

} // namespace Enemies
#endif // ENEMY_WASP_HPP