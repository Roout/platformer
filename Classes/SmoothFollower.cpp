#include "SmoothFollower.hpp"
#include "PhysicsHelper.hpp"
#include "units/Unit.hpp"

#include "cocos2d.h"

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
    const auto mapSize { tileMap->getContentSize() };
    const auto currentNodePos { tileMap->getPosition() };

    const auto visible = cocos2d::Director::getInstance()->getVisibleSize();
    const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
    
    const cocos2d::Rect bounds { 
        visible - mapSize - cocos2d::Size{ origin.x, origin.y }, 
        mapSize - visible + cocos2d::Size{ 2.f * origin.x, 2.f * origin.y }
    };
    const cocos2d::Vec2 before { this->x, this->y };

    auto target { m_unit->getPosition() };

    static constexpr auto alpha { 0.3f }; 
    static constexpr auto eps { 1.f }; 
    // depends on how high/low is the player
    const auto betta = target.y / visible.height;
    // this shift define how much it's shown below/above player.
    // The higher is the player the less he can see above himself and 
    // more below
    const auto shift = cocos2d::Vec2{0.f, 10.f - 40.f * betta };

    // *this * (1.f - alpha) + other * alpha;
    // a * (1 - x) + b * x = a - ax + bx = x(b - a) + a
    const auto pos = this->lerp(target, alpha) + shift;
    auto delta = m_delta;
    
    if (pos.fuzzyEquals(target, eps)) {
        m_delta = target - *this;
        x = target.x; 
        y = target.y;
    } 
    else {
        m_delta = pos - *this;
        x = pos.x; 
        y = pos.y;
    }
    auto newPosition { currentNodePos - m_delta };
    if(!helper::IsBetween(newPosition.x, bounds.getMinX(), bounds.getMaxX())) {
        newPosition.x = currentNodePos.x;
        m_delta.x = 0.f;
        x = before.x;
    }
    if(!helper::IsBetween(newPosition.y, bounds.getMinY(), bounds.getMaxY())) {
        newPosition.y = currentNodePos.y;
        m_delta.y = 0.f;
        y = before.y;
    }

    tileMap->setPosition(newPosition);
}

void SmoothFollower::Reset() {
    const auto position { m_unit->getPosition() };
    // set up follower position
    x = position.x;
    y = position.y;

    m_delta = { 0.f, 0.f };
}