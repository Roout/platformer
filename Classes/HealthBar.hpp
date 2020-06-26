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
    static HealthBar * create( const std::shared_ptr<Unit>& model);

    [[nodiscard]] bool init() override ;

    void update(float dt) override;

private:
    HealthBar(const std::shared_ptr<Unit>& model);

private:
    std::weak_ptr<Unit> m_model;

    int m_maxHealth { 0 };
};

#endif // HEALTH_BAR_HPP