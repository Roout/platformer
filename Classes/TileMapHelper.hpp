#ifndef TILE_MAP_HELPER_HPP
#define TILE_MAP_HELPER_HPP

#include <vector>
#include "cocos/math/CCGeometry.h"  // cocos2d::Size, cocos2d::Vec2
#include "Utils.hpp"

namespace cocos2d {
    class FastTMXTiledMap;
    class FastTMXLayer;
}

namespace TileMap {

	using Mask = uint8_t;

	enum class Property : uint8_t {
		BORDER 		= 0x1,
		EMPTY  		= 0x2, // no tile in placeholder
		OUTSIDE_MAP = 0x4,
		SOLID 		= 0x8,
		SPIKE 		= 0x10,
		PLATFORM 	= 0x20,
		UNDEFINED 	= 0x40
	};

	static const char* COLLISION_LAYER_NAME = "padding-background";

	struct Cache {

		const cocos2d::FastTMXTiledMap * tileMap { nullptr };
		cocos2d::FastTMXLayer * blocksLayer { nullptr };
		const cocos2d::Size tileSize { 0.f, 0.f }; // it's square
		const size_t mapWidth { 0U };
		const size_t mapHeight { 0U };
		std::vector<std::vector<Mask>> properties;

		Cache(const cocos2d::FastTMXTiledMap * tilemap);
		
		bool IsInMap(const cocos2d::Vec2& pos) const noexcept {
			return pos.x >= 0.f 
				&& pos.y >= 0.f 
				&& pos.x < static_cast<float>(mapWidth) 
				&& pos.y < static_cast<float>(mapHeight); 
		}

		Mask GetProperties(const cocos2d::Vec2& pos) const noexcept {
            if(!this->IsInMap(pos)) {
                return static_cast<Mask>(Property::OUTSIDE_MAP);
            }
			return properties[static_cast<size_t>(pos.y)][static_cast<size_t>(pos.x)];
		}

		Mask GetProperties(int row, int col) const noexcept {
            if(!this->IsInMap({ static_cast<float>(col), static_cast<float>(row) })) {
                return static_cast<Mask>(Property::OUTSIDE_MAP);
            }
			return properties[row][col];
		}

	private:
		Mask FindProperties(const cocos2d::Vec2& pos) const noexcept;
	};
}

#endif // TILE_MAP_HELPER_HPP