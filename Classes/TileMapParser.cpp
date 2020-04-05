#include "TileMapParser.hpp"
#include "cocos2d.h"

TileMapParser::TileMapParser(const cocos2d::FastTMXTiledMap * tileMap):
    m_tileMap{ tileMap }
{
    static constexpr auto expectedStaticBodiesCount { 50 };
    this->Get<ParsedType::STATIC_BODIES>().reserve(expectedStaticBodiesCount);
}

void TileMapParser::Parse() {
    // read player position from the map
    const auto group = m_tileMap->getObjectGroup("objects");
	if (group) {
		const auto& allObjects = group->getObjects();
		for (const auto& object : allObjects) {
			const auto& objMap = object.asValueMap();
			const auto type = objMap.at("type").asString();
			const auto name = objMap.at("name").asString();
			// TODO: change to object names
			if (type == "point") {
				const auto x = objMap.at("x").asFloat();
				const auto y = objMap.at("y").asFloat();
				if (name == "player") {
					this->Get<ParsedType::HERO_POSITION>() = cocos2d::Vec2{x, y};
				}
			}
		}
	}

    const auto obstaclesLayer = m_tileMap->getLayer("ground");
	if (obstaclesLayer) {
		const auto tileSize { obstaclesLayer->getMapTileSize() };
		const auto mapSize { obstaclesLayer->getLayerSize() };
		const auto width { static_cast<int>(mapSize.width) };
		const auto height { static_cast<int>(mapSize.height) };
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				const auto tileGid = obstaclesLayer->getTileGIDAt({
					static_cast<float>(i),
					static_cast<float>(j) 
				});
				if (tileGid) {
					const auto& properties = m_tileMap->getPropertiesForGID(tileGid).asValueMap();
					const bool isCollidable {
                        properties.count("collidable") > 0 && 
                        properties.at("collidable").asBool()
                    };
					if (isCollidable) {
                        this->Get<ParsedType::STATIC_BODIES>().emplace_back(
                            cocos2d::Vec2{ i * tileSize.width, (height - j - 1) * tileSize.height }, 
                            cocos2d::Size{ tileSize.width, tileSize.height }
                        );
					}

				}
			}
		}
	}
}