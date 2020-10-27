#include "TileMapHelper.hpp"

#include "cocos2d.h"
#include <cassert>

namespace TileMap {

Cache::Cache(const cocos2d::FastTMXTiledMap * tilemap) 
    : tileMap{ tilemap }
    , blocksLayer { tilemap->getLayer(COLLISION_LAYER_NAME) }
    , tileSize{ blocksLayer->getMapTileSize() }
    , mapWidth { static_cast<size_t>(blocksLayer->getLayerSize().width) }
    , mapHeight { static_cast<size_t>(blocksLayer->getLayerSize().height) }
    , properties { mapHeight, std::vector<Mask>(mapWidth, 0U) }
{
    // TODO: heavy calculations
    for(size_t row = 0; row < mapHeight; ++row) {
        for(size_t col = 0; col < mapWidth; ++col) {
            properties[row][col] = this->FindProperties({
                static_cast<float>(col), 
                static_cast<float>(row)
            });
        }
    }
}

Mask Cache::FindProperties(const cocos2d::Vec2& pos) const noexcept {
    Mask mask { 0U };
   
    const auto tileGid = blocksLayer->getTileGIDAt(pos);
    if(!tileGid) {
        mask = static_cast<Mask>(Property::EMPTY);
        return mask;
    }
    const auto tileProp { tileMap->getPropertiesForGID(tileGid) };
    const auto& properties { tileProp.asValueMap() };
    const auto categoryNameIter { properties.find("category-name") };

    assert(categoryNameIter != properties.end() && "Can't find property: <category-name>" );

    auto name = categoryNameIter->second.asString();
    if(name == "border") {
        mask = static_cast<Mask>(Property::BORDER); 
    }
    else if(name == "solid") {
        mask = static_cast<Mask>(Property::SOLID);
    }
    else if(name == "spikes") {
        mask = static_cast<Mask>(Property::SPIKE);
    }
    else if(name == "platform") {
        mask = static_cast<Mask>(Property::PLATFORM);
    }
    else {
        mask = static_cast<Mask>(Property::UNDEFINED);
    }
    return mask;
}

}