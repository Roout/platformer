#ifndef TILE_MAP_PARSER_HPP
#define TILE_MAP_PARSER_HPP

#include <vector>
#include <tuple>
#include "math/CCGeometry.h" // cocos2d::Rect, cocos2d::Vec2

namespace cocos2d {
    class FastTMXTiledMap;
}

enum class ParsedType: short {
    STATIC_BODIES,
    HERO_POSITION
};

/** @brief
 * Parse layers of the cocos2d::FastTMXTiledMap 
*/
class TileMapParser final {
public:

    TileMapParser(const cocos2d::FastTMXTiledMap * tilemap);

    void Parse();

    template <ParsedType Type>
    [[nodiscard]] auto&& Acquire() noexcept {
        return Get<Type>(std::move(m_parsed));
    }

private:
    template <ParsedType Type, class Tuple>
    [[nodiscard]] static auto&& Get(Tuple&& parsed) noexcept {
        return std::get<static_cast<short>(Type)>(std::forward<Tuple>(parsed));
    }

    template <ParsedType Type>
    [[nodiscard]] auto& Get() noexcept {
        return Get<Type>(m_parsed);
    }

    const cocos2d::FastTMXTiledMap * const m_tileMap { nullptr };
    // Parsed:
    std::tuple< 
        std::vector<cocos2d::Rect>,
        cocos2d::Vec2
    > m_parsed {};
};

#endif // TILE_MAP_PARSER_HPP