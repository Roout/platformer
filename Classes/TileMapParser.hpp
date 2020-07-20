#ifndef TILE_MAP_PARSER_HPP
#define TILE_MAP_PARSER_HPP

#include <vector>
#include <array>
#include "math/CCGeometry.h" // cocos2d::Rect, cocos2d::Vec2
#include "Core.hpp"
#include "Utils.hpp"

namespace cocos2d {
    class FastTMXTiledMap;
}

namespace details {
    
    struct Form final {
        core::CategoryName  m_type;
        std::pair<int,int>  m_position;
        cocos2d::Vec2       m_botLeft;
        cocos2d::Vec2       m_topRight;
        cocos2d::Rect       m_rect;
    };
}

/** @brief
 * Parse layers of the cocos2d::FastTMXTiledMap 
*/
class TileMapParser final {
public:
    using CategoryName = core::CategoryName;

    TileMapParser(const cocos2d::FastTMXTiledMap * tilemap);

    void Parse();

    template <CategoryName category>
    [[nodiscard]] auto&& Acquire() noexcept {
        return std::move(m_parsed[core::EnumCast(category)]);
    }

    [[nodiscard]] auto&& Acquire(CategoryName category) noexcept {
        return std::move(m_parsed[Utils::EnumCast(category)]);
    }

private:

    template <CategoryName category>
    [[nodiscard]] auto& Get() noexcept {
        return m_parsed[Utils::EnumCast(category)];
    }

    [[nodiscard]] auto& Get(CategoryName category) noexcept {
        return m_parsed[Utils::EnumCast(category)];
    }

    const cocos2d::FastTMXTiledMap * const m_tileMap { nullptr };
    
    std::array<
        std::vector<details::Form>, 
        Utils::EnumSize<CategoryName>()
    >  m_parsed;
};

#endif // TILE_MAP_PARSER_HPP