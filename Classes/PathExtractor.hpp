#ifndef PATH_EXTRACTOR_HPP
#define PATH_EXTRACTOR_HPP

#include "PathNodes.hpp"
#include "cocos2d.h"
#include <vector>

namespace Enemies {
    class Bot;
}

namespace path {

    /**
     * It's a tool to extract from the set of pathes a single path. 
     */
    class PathExtractor final {
    public:
        PathExtractor(
            const path::PathSet * pathes, 
            const cocos2d::FastTMXTiledMap * tilemap
        );

        path::Path ExtractPathFor(const Enemies::Bot* bot);

    private:
        void Dfs(
            size_t index, 
            std::vector<size_t>& connectedNodes, 
            std::vector<char>& visited
        ) const;
        
        cocos2d::Vec2 ReflectOrdinate(const cocos2d::Vec2& p) const noexcept {
            return {p.x, m_mapHeight - p.y - 1.f};
        }

        size_t FindClosestNodeIndex(const cocos2d::Vec2& position) const noexcept;

        size_t GetIndexOf(size_t value, const std::vector<size_t>& container) const noexcept;

        /// Properties

        /// external
        float m_tileSize { 0.f };
        float m_mapHeight { 0.f };
        const path::PathSet *m_pathes { nullptr };
    };

}

#endif // PATH_EXTRACTOR_HPP