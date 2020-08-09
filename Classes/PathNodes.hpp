#ifndef PATH_NODES_HPP
#define PATH_NODES_HPP

#include <vector>
#include "cocos2d.h" 

#include "rapidjson/document.h"

namespace path {
	/**
	 * Define action that must to be made to reach certain waypoint. 
	 */
	enum class Action {
		move, jump
	};

	/**
	 * This class keeps set of waypoints which belong to the level. 
	 * It also keeps adjacency matrix which indicates where unit can move/jump 
	 * from certain position.
	 * As the graph isn't connected, it's just number of trees i.e. forest.  
	 */
	struct Forest {
		std::vector<std::pair<int,int>> waypoints;
		std::vector<std::vector<std::pair<int, Action>>> adj;
    
		rapidjson::Document Load(size_t id);

		void Parse(rapidjson::Document& doc);

	private:
		const char* pathTemplate = "Map/supplement/level_%d/waypoints.json";
	};
}

#endif // PATH_NODES_HPP