#ifndef PATH_HPP
#define PATH_HPP

#include <vector>
#include <cinttypes>

#include "cocos/math/Vec2.h"

/**
 * Represent connected path for the single entity (unit)
 */
struct Path final {
	/// Properties

	// Number of raw points on the map parsed from polyline
	std::vector<cocos2d::Vec2> m_waypoints;
	size_t m_id { 0 };

	/// Lifecycle
	Path() = default;
	~Path() = default;
	
	Path(Path&&) = default;
	Path& operator=(Path&&) = default;

	Path(const Path&) = delete;
	Path& operator=(const Path&) = delete;
};

#endif // PATH_HPP