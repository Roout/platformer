#ifndef NAVIGATOR_HPP
#define NAVIGATOR_HPP

#include <limits>

#include "cocos2d.h"
#include "PathNodes.hpp"

class Unit;
namespace Enemies {
    class Bot;
}

class Navigator {
public:
    enum class Mode {
        PATROL,
        PURSUIT
    };

    Navigator(Enemies::Bot * owner, path::Path&& path);

    void Navigate(const float dt);

    void Pursue(Unit * const target) noexcept;

    void Patrol() noexcept;

    /// helper methods
private:

    // @p is tile's coordinates 
    size_t FindClosestPoint(const cocos2d::Vec2& p) const;

    // get your destination if possible
    // assume that the point always exist
    std::pair<size_t, path::Action> FindDestination(size_t from);

    bool ReachedDestination() const noexcept;
    
private:
    /// external data
    Enemies::Bot * m_owner { nullptr };
    const path::Path m_path;
    Unit * m_target { nullptr }; // target of the pursuit if exist

    /// internal data
    size_t m_start;
    size_t m_destination;
    path::Action m_action { path::Action::MOVE };
    Mode m_mode { Mode::PATROL };

    static constexpr size_t failure { std::numeric_limits<size_t>::max() };
};

#endif // NAVIGATOR_HPP