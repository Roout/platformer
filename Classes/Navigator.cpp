#include "Navigator.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"

#include <utility>
#include <tuple>

Navigator::Navigator(const cocos2d::Size& mapSize, float tileSize):
    m_mapHeight { static_cast<size_t>(mapSize.height) },
    m_tileSize{ tileSize }
{}

void Navigator::Init(Unit* const unit, path::Supplement * const forest) {
    m_unit = unit;
    m_supplement = forest;
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
        static_cast<float>(m_mapHeight - y - 1) // invert
    };
}

// @p is tile's coordinates 
size_t Navigator::FindClosestWaypoint(const cocos2d::Vec2& p) const {
    size_t closest { failure };
    size_t index { 0 };
    auto distance { std::numeric_limits<float>::max() };
    for(const auto& tile: m_supplement->waypoints ) {
        const auto inverted = this->AsInvertedTilemapCoords(tile.first, tile.second);
        const auto dist { fabs(p.x - inverted.x) + fabs(p.y - inverted.y) };
        if(dist < distance) {
            distance = dist;
            closest = index;
        }
        index++;
    } 
    return closest;
}

void Navigator::Navigate(const float dt) {
    if(m_target && m_target->IsDead()) {
        this->Patrol();
    }

    if( m_mode == Mode::pursue ) {
        const auto targetWidth = m_target->getContentSize().width / 2.f; 
        const auto unitWidth = m_unit->getContentSize().width / 2.f; 
        const auto dx = m_target->getPosition().x - m_unit->getPosition().x;
        auto destination { m_target->getPosition().x };
        if( dx < 0.f ) {
            // target -> unit
            destination += targetWidth + unitWidth;
        } 
        else {
            // unit -> target
            destination -= targetWidth + unitWidth;
        }
        // invoke move function
        if(helper::IsEquel(destination, m_unit->getPosition().x, 1.f)) {
            m_unit->Stop();
        } 
        else if( destination < m_unit->getPosition().x ) {
            m_unit->MoveLeft();
            if(!m_unit->IsLookingLeft()) {
                m_unit->Turn();
            }
        } 
        else {
            m_unit->MoveRight();
            if(m_unit->IsLookingLeft()) {
                m_unit->Turn();
            }
        }
        return;
    }
    // if destination is reached => 
        // search new one
    if(this->ReachedDestination()) {
        // make current destination point as new start
        m_start = m_destination;
        std::tie(m_destination, m_action) = this->FindDestination(m_start);
        m_unit->Stop();
    }
    // determine direction where unit should move
    // TODO: temporary only along X-axis
    const auto dx = (m_supplement->waypoints[m_destination].first + 0.5f) * m_tileSize - m_unit->getPosition().x;
    // invoke move function
    if( dx < 0.f ) {
        m_unit->MoveLeft();
        if(!m_unit->IsLookingLeft()) m_unit->Turn();
    } 
    else {
        m_unit->MoveRight();
        if(m_unit->IsLookingLeft()) m_unit->Turn();
    }
}

std::pair<size_t, path::Action> Navigator::FindDestination(size_t from) {
    const auto start { m_supplement->waypoints[from] };
    const auto& neighbours { m_supplement->adj[from] };
    // choose first for now
    return (neighbours.empty()? std::make_pair(from, path::Action::move): neighbours.front());
}

bool Navigator::ReachedDestination() const noexcept {
    cocos2d::Vec2 destination {};
    if( m_mode == Mode::patrol ) {
        destination = this->AsInvertedTilemapCoords(
            m_supplement->waypoints[m_destination].first, 
            m_supplement->waypoints[m_destination].second
        );
        destination.x += 0.5f; // tile's bottom mid
        destination *= m_tileSize; // from tiles to real coordinates where {0,0} is left-bot of the tilemap
    }
    else { // Mode::pursue
        /// TODO: if player's unit (i.e. out target) is dead? Is i possible?
        destination = m_target->getPosition(); // bot-mid
    }
    
    return fabs(destination.x - m_unit->getPosition().x) <= 8.f;// sensor.containsPoint( destinationPoint );
}

void Navigator::Pursue(Unit * const target) noexcept {
    m_mode = Mode::pursue;
    m_target = target;
    m_target->retain();
}

void Navigator::Patrol() noexcept {
    m_mode = Mode::patrol;
    m_target->release();
    m_target = nullptr;
}