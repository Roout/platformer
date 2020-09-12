#ifndef PATH_NODES_HPP
#define PATH_NODES_HPP

#include <vector>
#include <unordered_map>
#include <cinttypes>

#include "cocos2d.h" 

#include "rapidjson/document.h"

namespace path {
	/**
	 * Define action that must to be made to reach certain waypoint. 
	 */
	enum class Action : std::uint8_t {
		MOVE, 
		JUMP
	};

	using AdjacencyMatrix = std::vector<std::vector<std::pair<size_t, Action>>>;

	/**
	 * Represent number of pathes which belongs to singe level and many units
	 * 
	 * This class keeps set of waypoints which belong to the level. 
	 * It also keeps adjacency matrix which indicates where unit can move/jump 
	 * from certain position.
	 * As the graph isn't connected, it's just number of trees i.e. forest.  
	 */
	struct PathSet final {
		std::vector<cocos2d::Vec2> waypoints;
		AdjacencyMatrix adjacency;

		rapidjson::Document Load(size_t id);

		void Parse(rapidjson::Document& doc);

	private:
		const char* pathTemplate = "Map/supplement/level_%d/waypoints.json";
	};

	/**
	 * Represent connected path for the singe entity (unit)
	 */
	struct Path final {
		/// Properties

		// Already reflected along Y-axis points which belongs to this connected path
		// Keep real positions (not tile number but bottom-middle coordinate of the tile!) 
		// Ex. reflected tile with coordinate { col: 5, row 10 } will be 
		// { x: 5 * tileSize, y: 10 * tileSize } + { tileSize / 2.f, 0.f }
		std::vector<cocos2d::Vec2> m_waypoints;
		// adjacency matrix where `m_neighbours[index]` means get all indexes of neghbours 
		// of the node with tile coordinates: `m_waypoints[index]`
		path::AdjacencyMatrix m_neighbours;

		/// Lifecycle
		Path() = default;
		~Path() = default;
		
		Path(Path&&) = default;
		Path& operator=(Path&&) = default;

		Path(const Path&) = delete;
		Path& operator=(const Path&) = delete;
	};
}

#endif // PATH_NODES_HPP