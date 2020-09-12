#include "Navigator.hpp"
#include "Unit.hpp"
#include "PhysicsHelper.hpp"
#include "Bot.hpp"

#include <utility>
#include <tuple>

Navigator::Navigator(Enemies::Bot * owner, path::Path&& path) :
    m_owner { owner },
    m_path { std::move(path) }
{
    m_start = this->FindClosestPoint(m_owner->getPosition());
    if( m_start == failure ) {
        // TODO: oh no!
    }
    std::tie(m_destination, m_action) = this->FindDestination(m_start);
}

size_t Navigator::FindClosestPoint(const cocos2d::Vec2& p) const {
    size_t closest { failure };
    size_t pointIndex { 0 };
    auto distance { std::numeric_limits<float>::max() };
    for(const auto& point: m_path.m_waypoints ) {
        const auto dist { fabs(p.x - point.x) + fabs(p.y - point.y) };
        if(dist < distance) {
            distance = dist;
            closest = pointIndex;
        }
        ++pointIndex;
    } 
    return closest;
}

void Navigator::Navigate(const float dt) {
    if(m_target && m_target->IsDead()) {
        this->Patrol();
    }

    if( m_mode == Mode::PURSUIT ) {
        const auto targetWidth = m_target->getContentSize().width / 2.f; 
        const auto ownerWidth = m_owner->getContentSize().width / 2.f; 
        const auto dx = m_target->getPosition().x - m_owner->getPosition().x;
        auto destination { m_target->getPosition().x };
        if( dx < 0.f ) {
            // target -> unit
            destination += targetWidth + ownerWidth;
        } 
        else {
            // unit -> target
            destination -= targetWidth + ownerWidth;
        }
        // invoke move function
        if(helper::IsEquel(destination, m_owner->getPosition().x, 1.f)) {
            m_owner->Stop();
        } 
        else if( destination < m_owner->getPosition().x ) {
            m_owner->MoveLeft();
            if(!m_owner->IsLookingLeft()) {
                m_owner->Turn();
            }
        } 
        else {
            m_owner->MoveRight();
            if(m_owner->IsLookingLeft()) {
                m_owner->Turn();
            }
        }
        return;
    }
    // if destination is reached => search new one
    if(this->ReachedDestination()) {
        // make current destination point as new start
        m_start = m_destination;
        std::tie(m_destination, m_action) = this->FindDestination(m_start);
        m_owner->Stop();
    }
    // determine direction where unit should move
    // TODO: temporary only along X-axis
    const auto dx = m_path.m_waypoints[m_destination].x - m_owner->getPosition().x;
    // invoke move function
    if( dx < 0.f ) {
        m_owner->MoveLeft();
        if(!m_owner->IsLookingLeft()) {
            m_owner->Turn();
        }
    } 
    else {
        m_owner->MoveRight();
        if(m_owner->IsLookingLeft()) {
            m_owner->Turn();
        }
    }
}

std::pair<size_t, path::Action> Navigator::FindDestination(size_t from) {
    const auto start { m_path.m_waypoints[from] };
    const auto& neighbours { m_path.m_neighbours[from] };
    // choose first for now
    return (neighbours.empty()? std::make_pair(from, path::Action::MOVE): neighbours.front());
}

bool Navigator::ReachedDestination() const noexcept {
    cocos2d::Vec2 destination {};
    if( m_mode == Mode::PATROL ) {
        destination = m_path.m_waypoints[m_destination];
    }
    else { // Mode::pursue
        /// TODO: if player's unit (i.e. out target) is dead? Is i possible?
        destination = m_target->getPosition(); // bot-mid
    }
    
    return fabs(destination.x - m_owner->getPosition().x) <= 8.f;
}

void Navigator::Pursue(Unit * const target) noexcept {
    m_mode = Mode::PURSUIT;
    m_target = target;
    m_target->retain();
}

void Navigator::Patrol() noexcept {
    m_mode = Mode::PATROL;
    if(m_target) {
        m_target->release();
        m_target = nullptr;
    }
}