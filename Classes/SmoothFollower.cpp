#include "SmoothFollower.hpp"
#include "PhysicsHelper.hpp"
#include "Unit.hpp"

#include <cmath>

SmoothFollower::SmoothFollower( Unit * const unit ) :
    m_unit { unit }
{
    this->Reset();
}

/**
 * Smooth move from the current position to the new unit's position. 
 */
void SmoothFollower::UpdateMapPosition(const float dt) {
    const auto tileMap { m_unit->getParent() };
    const auto currentNodePos { tileMap->getPosition() };

    auto target { m_unit->getPosition() };

    static constexpr auto alpha { 0.3f }; 
    static constexpr auto eps { 1.f }; 
    // depends on how high/low is the player
    const auto betta = target.y / tileMap->getContentSize().height;
    // this shift define how much it's shown below/above player.
    // The higher is the player the less he can see above himself and 
    // more below
    const auto shift = cocos2d::Vec2{ 0.f, 10.f - 40.f * betta };

    // *this * (1.f - alpha) + other * alpha;
    // a * (1 - x) + b * x = a - ax + bx = x(b - a) + a
    const auto pos = this->lerp(target, alpha) + shift;
    if (pos.fuzzyEquals(target, eps)) {
        m_delta = target - *this;
        x = target.x, y = target.y;
    } 
    else {
        m_delta = pos - *this;
        x = pos.x, y = pos.y;
    }

    tileMap->setPosition(currentNodePos - m_delta);
}

void SmoothFollower::Reset() {
    const auto position { m_unit->getPosition() };
    // set up follower position
    x = position.x;
    y = position.y;

    m_delta = { 0.f, 0.f };
}