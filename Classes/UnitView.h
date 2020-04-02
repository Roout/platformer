#ifndef UNIT_VIEW_H
#define UNIT_VIEW_H

#include "cocos2d.h"

class Unit;

class HeroView final : public cocos2d::DrawNode {
public:
    static HeroView * create(const Unit* const model);

    [[nodiscard]] bool init() override;

    void update([[maybe_unused]] float dt) override;

private:
    HeroView(const Unit* model);

    const Unit * const m_model { nullptr };
};

#endif // UNIT_VIEW_H