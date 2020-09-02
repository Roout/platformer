#include "TileMapParser.hpp"
#include "cocos2d.h"

namespace {
	core::EnemyClass AsEnemyClass(const std::string& ty) noexcept {
		auto type { core::EnemyClass::UNDEFINED };
		if( ty == "warrior" ) {
			type = core::EnemyClass::WARRIOR;
		} 
		else if ( ty == "archer" ) {
			type = core::EnemyClass::ARCHER;
		}
		else if ( ty == "spear_man") {
			type = core::EnemyClass::SPEARMAN;
		}
		return type;
	}
}

TileMapParser::TileMapParser(const cocos2d::FastTMXTiledMap * tileMap):
    m_tileMap{ tileMap }
{
    this->Get<core::CategoryName::PLATFORM>().reserve(20);
	this->Get<core::CategoryName::BORDER>().reserve(100);
	this->Get<core::CategoryName::BARREL>().reserve(10);
	this->Get<core::CategoryName::ENEMY>().reserve(30);
	this->Get<core::CategoryName::INFLUENCE>().reserve(30);
	this->Get<core::CategoryName::SPIKES>().reserve(10);
}

void TileMapParser::Parse() {
    const auto group = m_tileMap->getObjectGroup("objects");
	if (group) {
		const auto& allObjects = group->getObjects();
		for (const auto& object : allObjects) {
			const auto& objMap = object.asValueMap();
			const auto type = objMap.at("type").asString();
			const auto name = objMap.at("name").asString();
			const auto x = objMap.at("x").asFloat();
			const auto y = objMap.at("y").asFloat();

			details::Form form;
			form.m_botLeft = cocos2d::Vec2{x, y};

			if( name == "player" ) {
				form.m_type = core::CategoryName::PLAYER;
				this->Get<core::CategoryName::PLAYER>().emplace_back(form);
			}
			else if( name == "barrel" ) {
				form.m_type = core::CategoryName::BARREL;
				this->Get<core::CategoryName::BARREL>().emplace_back(form);
			}
			else if( name == "enemy" ) {
				form.m_type = core::CategoryName::ENEMY;
				form.m_enemyClass = ::AsEnemyClass(type);
				form.m_id = objMap.at("id").asUnsignedInt();
				this->Get<core::CategoryName::ENEMY>().emplace_back(form);
			}
			else if( name == "Influence" ) {
				form.m_type = core::CategoryName::INFLUENCE;
				form.m_id = objMap.at("owner-id").asUnsignedInt();
				cocos2d::Size size {
					objMap.at("width").asFloat(),
					objMap.at("height").asFloat()
				};
				form.m_rect = cocos2d::Rect{ form.m_botLeft, size };
				this->Get<core::CategoryName::INFLUENCE>().emplace_back(form);
			}
		}
	}

    const auto obstaclesLayer = m_tileMap->getLayer("ground");
	if (obstaclesLayer) {
		const auto tileSize { obstaclesLayer->getMapTileSize() };
		const auto mapSize { obstaclesLayer->getLayerSize() };
		const auto width { static_cast<int>(mapSize.width) };
		const auto height { static_cast<int>(mapSize.height) };
		// check whether the tile was already visited
		// for now I don't need it.
    	std::vector<std::vector<char>> isVisited (height, std::vector<char>(width, false));
		
		auto GetTileInfo = [this](int tileGid) {
			auto info { std::make_pair(std::string(""), std::string("")) };
			
			if (tileGid) {
				const auto tileProp { m_tileMap->getPropertiesForGID(tileGid) };
				const auto& properties = tileProp.asValueMap();

				const auto bodyTypeIter = properties.find("body-type");
				const auto categoryNameIter = properties.find("category-name");

				const auto exist { 
					bodyTypeIter != properties.end() && 
					categoryNameIter != properties.end()
				};

				if( exist ) {
					info = std::make_pair(bodyTypeIter->second.asString(), categoryNameIter->second.asString());
				}
			}

			return info;
		};

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				const auto tileGid = obstaclesLayer->getTileGIDAt({
					static_cast<float>(x),
					static_cast<float>(y) 
				});

				if (tileGid) {
					// { bodyType, categoryName}
					const auto [srcBodyType, srcCategoryName] = GetTileInfo(tileGid);
					
					if(srcBodyType == "static" && !isVisited[y][x] ) {
						isVisited[y][x] = true;
						const auto category { core::CategoryFromString(srcCategoryName) };
						// Some bodies can consist from several tiles so
						// they should be combined into one physics body.

						// merge all horizontal neighbors:
						auto col { x + 1 };
						while(col < width && !isVisited[y][col] ) {
							// if the right tile is same (static & categoory)
							// then merge them
							const auto gid = obstaclesLayer->getTileGIDAt({
								static_cast<float>(col),
								static_cast<float>(y) 
							});
							if(!gid) break;
							
							if(	const auto [neighborBody, neighborCategory] = GetTileInfo(gid);
								neighborBody != srcBodyType || 
								srcCategoryName != neighborCategory
							) {
								break;
							}

							isVisited[y][col] = true;
							col++;
						}

						details::Form form;
						form.m_rect = cocos2d::Rect{
							cocos2d::Vec2{ x * tileSize.width, (height - y - 1) * tileSize.height }, 
							cocos2d::Size{ tileSize.width * (col - x), tileSize.height }
						};
						form.m_position = { x, y };
						form.m_type = category;

						this->Get(category).emplace_back(form);

						// now merge into vertical body if possible

						auto topRow { y + 1 };
						while(topRow < height && !isVisited[topRow][x] ) {
							// if the right tile is same (static & categoory)
							// then merge them
							const auto gid = obstaclesLayer->getTileGIDAt({
								static_cast<float>(x),
								static_cast<float>(topRow) 
							});
							if(!gid) break;
							
							if(	const auto& [neighborBody, neighborCategory] = GetTileInfo(gid);
								neighborBody != srcBodyType || 
								srcCategoryName != neighborCategory
							) {
								break;
							}

							isVisited[topRow][x] = true;
							topRow++;
						}
						topRow--;

						auto lowRow { y - 1 };
						while(lowRow >= 0 && !isVisited[lowRow][x] ) { 
							// if the right tile is same (static & categoory)
							// then merge them
							const auto gid = obstaclesLayer->getTileGIDAt({
								static_cast<float>(x),
								static_cast<float>(lowRow) 
							});
							if(!gid) break;
							
							if(	const auto& [neighborBody, neighborCategory] = GetTileInfo(gid);
								neighborBody != srcBodyType || 
								srcCategoryName != neighborCategory
							) {
								break;
							}

							isVisited[lowRow][x] = true;
							lowRow--;
						}
						lowRow++;

						if( lowRow != topRow ) { // at least one tile can be merged
							form.m_rect = cocos2d::Rect{
								cocos2d::Vec2{ x * tileSize.width, (height - topRow - 1.f) * tileSize.height }, 
								cocos2d::Size{ tileSize.width, tileSize.height * (topRow - lowRow + 1.f) }
							};
							form.m_position = { x, lowRow };
							form.m_type = category;
							this->Get(category).emplace_back(form);
						}

						// skip horizontal tiles if can
						x = col - 1;
					} 
				}	// tileGid
			}	// for
		}	// for
	}
}