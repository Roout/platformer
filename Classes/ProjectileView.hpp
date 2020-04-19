#ifndef PROJECTILE_VIEW_HPP
#define PROJECTILE_VIEW_HPP

#include "cocos2d.h"

class Projectile;

class ProjectileView final : public cocos2d::DrawNode {
public:
    static ProjectileView * create(const Projectile* const model);

    [[nodiscard]] bool init() override;

    void update([[maybe_unused]] float dt) override;

private:
    ProjectileView(const Projectile* model);

    const Projectile * const m_model { nullptr };
};

#endif // PROJECTILE_VIEW_HPP
