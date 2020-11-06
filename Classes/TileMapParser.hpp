#ifndef TILE_MAP_PARSER_HPP
#define TILE_MAP_PARSER_HPP

#include <vector>
#include <string>
#include <array>
#include <memory>
#include <unordered_map>
#include "math/CCGeometry.h" // cocos2d::Rect, cocos2d::Vec2
#include "Core.hpp"
#include "Utils.hpp"

namespace cocos2d {
    class FastTMXTiledMap;
}

namespace TileMap {
    struct Cache;
}

namespace details {
    
    struct Form final {
        core::CategoryName  m_type { core::CategoryName::UNDEFINED };
        size_t  m_subType { 0u };
        size_t  m_id { 0u };
        size_t  m_pathId { 0u };
        size_t  m_ownerId { 0u };
        float   m_scale { 0.f };
        /**
         * Front point always defines a position
         * If it's a Polygon than it contains all points of the figure
         */
        std::vector<cocos2d::Vec2> m_points;
        /**
         * Define rect object 
         */
        cocos2d::Rect m_rect;
    };

	using GID = size_t;

	struct TileSet final {
		GID 		firstgid;
		std::string name;
		uint16_t 	tilewidth;
		uint16_t 	tileheight;
		std::vector<std::pair<std::string, std::string>> properties;
	};
}

/** @brief
 * Parse layers of the cocos2d::FastTMXTiledMap 
*/
class TileMapParser final {
public:
    using CategoryName = core::CategoryName;

    TileMapParser(const cocos2d::FastTMXTiledMap * tilemap, const std::string& tmxFile);

    ~TileMapParser();

    void Parse();

    template <CategoryName category>
    [[nodiscard]] auto&& Acquire() noexcept {
        return std::move(m_parsed[core::EnumCast(category)]);
    }

    [[nodiscard]] auto&& Acquire(CategoryName category) noexcept {
        return std::move(m_parsed[Utils::EnumCast(category)]);
    }
    
    [[nodiscard]] const auto& Peek(CategoryName category) const noexcept {
        return m_parsed[Utils::EnumCast(category)];
    }
    
private:

    void ParseTileSets();

    void ParseUnits();

    void ParseProps();
    
    void ParsePaths();
    
    void ParseInfluences();

    template <CategoryName category>
    [[nodiscard]] auto& Get() noexcept {
        return m_parsed[Utils::EnumCast(category)];
    }

    [[nodiscard]] auto& Get(CategoryName category) noexcept {
        return m_parsed[Utils::EnumCast(category)];
    }

    const cocos2d::FastTMXTiledMap * const m_tileMap { nullptr };

    const std::string m_tmxFile;

    std::unique_ptr<const TileMap::Cache> m_tileMapCache;

    std::array<
        std::vector<details::Form>, 
        Utils::EnumSize<CategoryName>()
    >  m_parsed;

    
	std::unordered_map<details::GID, details::TileSet> m_tileSets;
};

#endif // TILE_MAP_PARSER_HPP