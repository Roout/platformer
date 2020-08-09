#include "Navigator.hpp"
#include "Unit.hpp"

#include <utility>
#include <tuple>

Navigator::Navigator(const cocos2d::Size& mapSize, float tileSize):
    m_mapHeight { static_cast<size_t>(mapSize.height) },
    m_tileSize{ tileSize }
{}

void Navigator::Init(Unit* const unit, path::Forest * const forest) {
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

cocos2d::Vec2 Navigator::AsInvertedTilemapCoords(int x, int y) const noexcept  {
    return {
        static_cast<float>(x), 
        static_cast<float>(m_mapHeight - y) // invert
    };
}

// @p is tile's coordinates 
size_t Navigator::FindClosestWaypoint(const cocos2d::Vec2& p) const {
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

std::pair<size_t, path::Action> Navigator::FindDestination(size_t from) {
    const auto start { m_forest->waypoints[from] };
    const auto& neighbours { m_forest->adj[from] };
    // choose first for now
    return (neighbours.empty()? std::make_pair(from, path::Action::move): neighbours.front());
}

void Navigator::Navigate(const float dt) {
    // if destination is reached => 
        // search new one
    if(this->ReachedDestination()) {
        // make current destination point as new start
        m_start = m_destination;
        std::tie(m_destination, m_action) = this->FindDestination(m_start);
        m_unit->GetMovement().StopXAxisMove();
    }
    // determine direction where unit should move
    // TODO: temporary only along X-axis
    const auto dx = (m_forest->waypoints[m_destination].first + 0.5f) * m_tileSize - m_unit->getPosition().x;
    // invoke move function
    if( dx < 0.f ) {
        m_unit->GetMovement().MoveLeft();
    } else {
        m_unit->GetMovement().MoveRight();
    }
}

bool Navigator::ReachedDestination() const noexcept {
    const float width { m_tileSize / 10.f };
    const cocos2d::Rect sensor { m_unit->getPosition(), { width, width }  };
    const auto destinationTile { m_forest->waypoints[m_destination] };
    const auto coords { this->AsInvertedTilemapCoords(destinationTile.first, destinationTile.second) };
    const auto destinationPoint {
        m_tileSize * (coords + cocos2d::Vec2{ 0.5f, 0.f })
    };
    return fabs(destinationPoint.x - m_unit->getPosition().x) <= 8.f;// sensor.containsPoint( destinationPoint );
}
