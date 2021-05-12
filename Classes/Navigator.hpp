#ifndef NAVIGATOR_HPP
#define NAVIGATOR_HPP

#include <limits>

#include "cocos2d.h"
#include "Path.hpp"

namespace Enemies {
    class Bot;
}

class Navigator {
public:

    Navigator(Enemies::Bot * owner, Path&& path);

    void Update(float dt);

    void VisitCustomPoint(const cocos2d::Vec2& destination);

    void FollowPath();

    void SetPrecision(float precision) noexcept;

    /// helper methods
private:
   
    void MoveTo(const cocos2d::Vec2& destination);

    size_t FindClosestPathPoint(const cocos2d::Vec2& p) const;

    // get your destination if possible
    // assume that the point always exist
    size_t FindDestination(size_t from);

    std::pair<bool, bool> ReachedDestination() const noexcept;

private:
    /// external data
    Enemies::Bot * const    m_owner { nullptr };
    cocos2d::Vec2           m_customTarget {0.f, 0.f};
    const Path              m_path;

    /// internal data
    size_t                  m_choosenWaypointIndex { 0 };
    bool                    m_isFollowingPath { true };
    float                   m_checkPrecision { 4.f };

    static constexpr size_t failure { std::numeric_limits<size_t>::max() };
};

#endif // NAVIGATOR_HPP