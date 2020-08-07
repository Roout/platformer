#ifndef BRAIN_HPP
#define BRAIN_HPP

#include <limits>

#include "cocos2d.h"
#include "EnemyPath.hpp"

class Unit;

class EnemyMoveComponent {
public:    
    EnemyMoveComponent(const cocos2d::Size& mapSize, float tileSize);

    void Init(Unit* const unit, path::Forest * const forest);

    void Navigate(const float dt);

    /// halper methods
private:
    // @p is tile's coordinates 
    size_t FindClosestWaypoint(const cocos2d::Vec2& p) const;

    // get your destination if possible
    // assume that the point always exist
    std::pair<size_t, path::Action> FindDestination(size_t from);

    bool ReachedDestination() const noexcept;
    
    cocos2d::Vec2 AsInvertedTilemapCoords(int x, int y) const noexcept;

private:
    /// external data
    Unit * m_unit { nullptr };
    path::Forest * m_forest { nullptr };

    size_t m_mapHeight { 0 }; // number of tiles
    float m_tileSize { 0.f };

    /// internal data
    size_t m_start;
    size_t m_destination;
    path::Action m_action { path::Action::move };

    static constexpr size_t failure { std::numeric_limits<size_t>::max() };
};

#include "Unit.hpp"

EnemyMoveComponent::EnemyMoveComponent(const cocos2d::Size& mapSize, float tileSize):
    m_mapHeight { static_cast<size_t>(mapSize.height) },
    m_tileSize{ tileSize }
{}

void EnemyMoveComponent::Init(Unit* const unit, path::Forest * const forest) {
    m_unit = unit;
    m_forest = forest;
    // initialize path points
    const auto unitTile { unit->getPosition() / m_tileSize };
    m_start = this->FindClosestWaypoint(unitTile);
    if( m_start == failure ) {
        // TODO: oh no!
    }
    std::tie(m_destination, m_action) = this->FindDestination(m_start);
}

cocos2d::Vec2 EnemyMoveComponent::AsInvertedTilemapCoords(int x, int y) const noexcept  {
    return {
        static_cast<float>(x), 
        static_cast<float>(m_mapHeight - y) // invert
    };
}

// @p is tile's coordinates 
size_t EnemyMoveComponent::FindClosestWaypoint(const cocos2d::Vec2& p) const {
    size_t closest { failure };
    size_t index { 0 };
    auto distance { std::numeric_limits<float>::max() };
    for(const auto& tile: m_forest->waypoints ) {
        const auto inverted = m_mapHeight - tile.second;
        const auto dist { fabs(p.x - tile.first) + fabs(p.y - inverted) };
        if(dist < distance) {
            distance = dist;
            closest = index;
        }
        index++;
    } 
    return closest;
}

// get your destination if possible
// assume that the point always exist
std::pair<size_t, path::Action> EnemyMoveComponent::FindDestination(size_t from) {
    const auto start { m_forest->waypoints[from] };
    const auto& neighbours { m_forest->adj[from] };
    // choose first for now
    return (neighbours.empty()? std::make_pair(from, path::Action::move): neighbours.front());
}

void EnemyMoveComponent::Navigate(const float dt) {
    // if destination is reached => 
        // search new one
    if(this->ReachedDestination()) {
        // make current destination point as new start
        m_start = m_destination;
        std::tie(m_destination, m_action) = this->FindDestination(m_start);
    }

    // determine direction where unit should move
    // TODO: temporary only along X-axis
    const auto dx = m_forest->waypoints[m_destination].first * m_tileSize - m_unit->getPosition().x;

    // invoke move function
    if( dx < 0.f ) {
        m_unit->GetMovement().MoveLeft();
    } else {
        m_unit->GetMovement().MoveRight();
    }
}

bool EnemyMoveComponent::ReachedDestination() const noexcept {
    const float width { m_tileSize / 10.f };
    cocos2d::Rect sensor { m_unit->getPosition(), {width, width}  };
    const auto destinationTile { m_forest->waypoints[m_destination] };
    const auto destinationPoint {
        this->AsInvertedTilemapCoords(destinationTile.first, destinationTile.second)
    };
    return sensor.containsPoint( destinationPoint );
}

#endif // BRAIN_HPP