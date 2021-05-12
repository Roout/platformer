#ifndef ENEMIES_SPEAR_MAN_HPP
#define ENEMIES_SPEAR_MAN_HPP

#include "Warrior.hpp"

namespace Enemies {

class Spearman : public Warrior {
public:
    static Spearman* create(size_t id, const cocos2d::Size& contentSize);

    bool init() override;

private:
    enum WeaponClass { MELEE };

    Spearman(size_t id, const char * name, const cocos2d::Size& contentSize);

    void AddWeapons() override;

    void Attack() override;
};

} // namespace Enemies

#endif // ENEMIES_SPEAR_MAN_HPP