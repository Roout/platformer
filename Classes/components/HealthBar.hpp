#ifndef HEALTH_BAR_HPP
#define HEALTH_BAR_HPP

#include <memory>
#include "cocos2d.h"

class Unit;

/**
 * Unit's health bar.
 * 
 * @code
 *  @endcode
 */
class HealthBar final : public cocos2d::DrawNode {
public:
    static HealthBar * create( const Unit * unit);

    [[nodiscard]] bool init() override ;

    void update(float dt) override;

private:
    HealthBar(const Unit * const unit);

private:
    const Unit * const m_unit { nullptr };

    int m_maxHealth { 0 };
};

#endif // HEALTH_BAR_HPP