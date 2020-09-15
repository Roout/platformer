#ifndef ENEMIES_SPEAR_MAN_HPP
#define ENEMIES_SPEAR_MAN_HPP

#include "Warrior.hpp"

namespace Enemies {

class Spearman : public Warrior {
public:
    static Spearman* Spearman::create(size_t id);

    bool init() override;

private:
    Spearman(size_t id, const char * name);

    void AddWeapon() override;

    void Attack() override;
};

} // namespace Enemies

#endif // ENEMIES_SPEAR_MAN_HPP