#include "TileMapParser.hpp"
#include "cocos2d.h"

TileMapParser::TileMapParser(const cocos2d::FastTMXTiledMap * tileMap):
    m_tileMap{ tileMap }
{
    static constexpr auto expectedStaticBodiesCount { 100 };
    static constexpr auto expectedKinematicBodiesCount { 10 };

    this->Get<ParsedType::STATIC_BODIES>().reserve(expectedStaticBodiesCount);
    this->Get<ParsedType::KINEMATIC_BODIES>().reserve(expectedKinematicBodiesCount);
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
					this->Get<ParsedType::PLAYER>() = cocos2d::Vec2{x, y};
				}
				else if ( name == "barrel") {
					this->Get<ParsedType::STATIC_BODIES>().emplace_back(
						cocos2d::Rect{
							cocos2d::Vec2{ x, y }, 
							cocos2d::Size{ 1.f, 1.f } // size doesn't matter cuz it's defined in the class
						},
						core::CategoryName::BARREL								
					);
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
					const auto tileProp { m_tileMap->getPropertiesForGID(tileGid) };
					const auto& properties = tileProp.asValueMap();

					const auto bodyTypeIter = properties.find("body-type");
					const auto categoryNameIter = properties.find("category-name");

					const auto exist { 
						bodyTypeIter != properties.end() && 
						categoryNameIter != properties.end()
					};

					if (!exist) continue;

					if(bodyTypeIter->second.asString() == "static") {
						core::CategoryName category { core::CategoryName::UNDEFINED };

						if( categoryNameIter->second.asString() == "platform" ) {
							category = core::CategoryName::PLATFORM;
						} 
						else if(categoryNameIter->second.asString() == "border") {
							category = core::CategoryName::BORDER;
						} 

						this->Get<ParsedType::STATIC_BODIES>().emplace_back(
							cocos2d::Rect{
								cocos2d::Vec2{ i * tileSize.width, (height - j - 1) * tileSize.height }, 
								cocos2d::Size{ tileSize.width, tileSize.height }
							},
							category								
						);
					} else { // "kinematic"
						core::CategoryName category { core::CategoryName::UNDEFINED };

						this->Get<ParsedType::KINEMATIC_BODIES>().emplace_back(
							cocos2d::Rect{
								cocos2d::Vec2{ i * tileSize.width, (height - j - 1) * tileSize.height }, 
								cocos2d::Size{ tileSize.width, tileSize.height }
							},
							category								
						);
					}

				}	// tileGid
			}	// for
		}	// for
	}
}