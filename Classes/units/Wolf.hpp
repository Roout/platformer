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
};

} // namespace Enemies

#endif // ENEMIES_WOLF_HPP