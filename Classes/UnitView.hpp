#ifndef UNIT_VIEW_HPP
#define UNIT_VIEW_HPP

#include "cocos2d.h"
#include "Unit.hpp"

class HeroView final : public cocos2d::DrawNode {
public:
    static HeroView * create(const Unit* const model);

    [[nodiscard]] bool init() override;

    void update([[maybe_unused]] float dt) override;

private:
    HeroView(const Unit* model);

    void UpdateAnimation();

    void FlipX(const Unit::Side);
private:

    const Unit * const m_model { nullptr };

    /**
     * Data from the previous unit state.
     * Used to monitor the changes of the unit state so that view will also be changed 
     * with animation and other stuff.
     */
    struct LastUpdate {
        Unit::Side   direction { Unit::Side::left };
        Unit::State     state { Unit::State::idle };
    } m_lastState {};
};

#endif // UNIT_VIEW_HPP