#ifndef NAVIGATOR_HPP
#define NAVIGATOR_HPP

#include <limits>

#include "cocos2d.h"
#include "PathNodes.hpp"

class Unit;

class Navigator {
public:    
    // mapSize in tiles
    Navigator(const cocos2d::Size& mapSize, float tileSize);

    void Init(Unit* const unit, path::Forest * const forest);

    void Navigate(const float dt);

    /// helper methods
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

#endif // NAVIGATOR_HPP