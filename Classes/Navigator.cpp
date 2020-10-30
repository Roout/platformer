#include "Navigator.hpp"
#include "PhysicsHelper.hpp"
#include "Bot.hpp"

#include <cassert>
#include <utility>

Navigator::Navigator(Enemies::Bot * owner, Path&& path) :
    m_owner { owner },
    m_path { std::move(path) }
{
    m_choosenWaypointIndex = this->FindClosestPathPoint(m_owner->getPosition());
    assert(m_choosenWaypointIndex != failure && "The path doesn't exist!");
}

void Navigator::Update(float dt) {
    auto target { m_customTarget };
    if(m_isFollowingPath) {
        if(const auto [reachedX, reachedY] = this->ReachedDestination(); reachedX && reachedY ) {
            m_choosenWaypointIndex = this->FindDestination(m_choosenWaypointIndex);
            m_owner->MoveAlong(0.f, 0.f);
        }
        target = m_path.m_waypoints[m_choosenWaypointIndex];
    }
    this->MoveTo(target);
}

void Navigator::VisitCustomPoint(const cocos2d::Vec2& destination) {
    m_customTarget = destination;
    m_isFollowingPath = false;
}

void Navigator::FollowPath() {
    m_isFollowingPath = true;
}

void Navigator::MoveTo(const cocos2d::Vec2& destination) {
    if(m_owner && !m_owner->IsDead()) {
        auto AsDirection = [](float x) {
            return x > 0.f? 1.f: -1.f;
        };
        const auto [reachedX, reachedY] = this->ReachedDestination();
        const auto vec { destination - m_owner->getPosition() };
        const cocos2d::Vec2 dir {
            reachedX? 0.f: AsDirection(vec.x), 
            reachedY? 0.f: AsDirection(vec.y)
        };
        if(dir.x != 0.f || dir.y != 0.f) { // ignore when the command is to stop (0.f, 0.f)
            m_owner->LookAt(destination);
        }
        m_owner->MoveAlong(dir);
    }       
}

size_t Navigator::FindDestination(size_t from) {
    // TODO: add some clever decision making algorithm.
    // but for now any path consist onlyu from 2 points so it doesn't matter
    return (from + 1) % m_path.m_waypoints.size();
}

std::pair<bool, bool> Navigator::ReachedDestination() const noexcept {
    cocos2d::Vec2 destination = m_customTarget;
    if(m_isFollowingPath) {
        destination = m_path.m_waypoints[m_choosenWaypointIndex];
    }
    constexpr auto checkPrecision { 4.f };
    const auto reachedX = fabs(destination.x - m_owner->getPosition().x) <= checkPrecision;
    const auto reachedY = fabs(destination.y - m_owner->getPosition().y) <= checkPrecision;
    return { reachedX, reachedY };
}

size_t Navigator::FindClosestPathPoint(const cocos2d::Vec2& position) const {
    size_t closest { failure };
    size_t pointIndex { 0 };
    auto distance { std::numeric_limits<float>::max() };
    for(const auto& point: m_path.m_waypoints) {
        const auto dist { position.getDistanceSq(point) };
        if(dist < distance) {
            distance = dist;
            closest = pointIndex;
        }
        ++pointIndex;
    } 
    return closest;
}

