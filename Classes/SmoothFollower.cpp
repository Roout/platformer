#include "SmoothFollower.hpp"
#include "Unit.hpp"

SmoothFollower::SmoothFollower( Unit * const unit ) :
    m_unit { unit }
{
    this->Reset();
}

/**
 * Smooth move from the current position to the new unit's position. 
 */
void SmoothFollower::UpdateMapPosition(const float dt) {
    const auto destination { m_unit->getPosition() };
    static constexpr auto alpha { 0.1f }; 
    static constexpr auto eps { 0.1f }; 

    const auto pos = this->lerp(destination, alpha);
    if( pos.fuzzyEquals(destination, eps)) {
        m_delta = destination - *this;
        x = destination.x, y = destination.y;
    } else {
        m_delta = pos - *this;
        x = pos.x, y = pos.y;
    }

    const auto tileMap { m_unit->getParent() };
    const auto currentNodePos { tileMap->getPosition()};
    tileMap->setPosition(currentNodePos - m_delta);
}

void SmoothFollower::Reset() {
    const auto position { m_unit->getPosition() };
    // set up follower position
    x = position.x;
    y = position.y;

    m_delta = { 0.f, 0.f };
}