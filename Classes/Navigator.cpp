#include "Navigator.hpp"
#include "Movement.hpp"
#include "PhysicsHelper.hpp"

#include "units/Bot.hpp"

#include <cassert>
#include <utility>

namespace {
    Movement::Direction GetHorizontalDirection(float x) noexcept {
        assert(x != 0.f);
        return x > 0.f? Movement::Direction::RIGHT
            : Movement::Direction::LEFT;
    }

    Movement::Direction GetVerticalDirection(float y) noexcept {
        assert(y != 0.f);
        return y > 0.f? Movement::Direction::UP
            : Movement::Direction::DOWN;
    }
} // namespace 

Navigator::Navigator(Enemies::Bot * owner, Path&& path) :
    m_owner { owner },
    m_path { std::move(path) }
{
    m_choosenWaypointIndex = this->FindClosestPathPoint(m_owner->getPosition());
    assert(m_choosenWaypointIndex != failure && "The path doesn't exist!");
}

void Navigator::Update(float dt) {
    auto target { m_customTarget };
    if (m_isFollowingPath) {
        if (auto [reachedX, reachedY] = ReachedDestination(); reachedX && reachedY) {
            m_choosenWaypointIndex = this->FindDestination(m_choosenWaypointIndex);
            m_owner->Stop(Movement::Axis::XY);
        }
        target = m_path.m_waypoints[m_choosenWaypointIndex];
    }
    MoveTo(target);
}

void Navigator::VisitCustomPoint(const cocos2d::Vec2& destination) {
    m_customTarget = destination;
    m_isFollowingPath = false;
}

void Navigator::FollowPath() {
    m_isFollowingPath = true;
}

void Navigator::MoveTo(const cocos2d::Vec2& destination) {
    if (m_owner && !m_owner->IsDead()) {
        const auto [reachedX, reachedY] = ReachedDestination();
        const auto direction { destination - m_owner->getPosition() };

        if (reachedX && reachedY) {
            m_owner->Stop(Movement::Axis::XY);
        }
        else if (reachedX) {
            m_owner->Stop(Movement::Axis::X);
            m_owner->MoveAlong(GetVerticalDirection(direction.y));
        }
        else if (reachedY) {
            m_owner->Stop(Movement::Axis::Y);
            m_owner->LookAt(destination);
            m_owner->MoveAlong(GetHorizontalDirection(direction.x));
        }
        else {
            m_owner->LookAt(destination);
            m_owner->MoveAlong(GetHorizontalDirection(direction.x)); // X-axis
            m_owner->MoveAlong(GetVerticalDirection(direction.y)); // Y-axis
        }
    }
}

size_t Navigator::FindDestination(size_t from) {
    // TODO: add some clever decision making algorithm.
    // but for now any path consist onlyu from 2 points so it doesn't matter
    return (from + 1) % m_path.m_waypoints.size();
}

std::pair<bool, bool> Navigator::ReachedDestination() const noexcept {
    cocos2d::Vec2 destination = m_isFollowingPath? m_path.m_waypoints[m_choosenWaypointIndex]: m_customTarget;
    const auto reachedX = fabs(destination.x - m_owner->getPosition().x) <= m_checkPrecision;
    const auto reachedY = fabs(destination.y - m_owner->getPosition().y) <= m_checkPrecision;
    return { reachedX, reachedY };
}

void Navigator::SetPrecision(float precision) noexcept {
    m_checkPrecision = precision;
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

