#include "TileMapParser.hpp"
#include "cocos2d.h"

namespace {
	core::EnemyClass AsEnemyClass(const std::string& ty) noexcept {
		auto type { core::EnemyClass::UNDEFINED };
		if( ty == core::EntityNames::WARRIOR ) {
			type = core::EnemyClass::WARRIOR;
		} 
		else if ( ty == core::EntityNames::ARCHER ) {
			type = core::EnemyClass::ARCHER;
		}
		else if ( ty == core::EntityNames::SPEARMAN ) {
			type = core::EnemyClass::SPEARMAN;
		}
		else if ( ty == core::EntityNames::SPIDER ) {
			type = core::EnemyClass::SPIDER;
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
	this->Get<core::CategoryName::PATH>().reserve(30);
	this->Get<core::CategoryName::INFLUENCE>().reserve(30);
	this->Get<core::CategoryName::SPIKES>().reserve(10);
}


void TileMapParser::ParseUnits() {
	const auto group = m_tileMap->getObjectGroup("units");
	if (group) {
		const auto& allObjects = group->getObjects();
		for (const auto& object : allObjects) {
			const auto& objMap = object.asValueMap();
			const auto type = objMap.at("type").asString();
			const auto name = objMap.at("name").asString();
			const auto x = objMap.at("x").asFloat();
			const auto y = objMap.at("y").asFloat();

			details::Form form;
			form.m_points.emplace_back(x, y);
			form.m_id = objMap.at("id").asUnsignedInt();

			if(name == "player") {
				form.m_type = core::CategoryName::PLAYER;
				this->Get<core::CategoryName::PLAYER>().emplace_back(form);
			}
			else if(name == "enemy") {
				form.m_type = core::CategoryName::ENEMY;
				form.m_pathId = objMap.at("path-id").asUnsignedInt();
				form.m_enemyClass = ::AsEnemyClass(type);
				this->Get<core::CategoryName::ENEMY>().emplace_back(form);
			}
		}
	}
}

void TileMapParser::ParseProps() {
	const auto group = m_tileMap->getObjectGroup("props");
	if (group) {
		const auto& allObjects = group->getObjects();
		for (const auto& object : allObjects) {
			const auto& objMap = object.asValueMap();
			const auto type = objMap.at("type").asString();
			const auto name = objMap.at("name").asString();
			const auto x = objMap.at("x").asFloat();
			const auto y = objMap.at("y").asFloat();

			details::Form form;
			form.m_points.emplace_back(x, y);
			form.m_id = objMap.at("id").asUnsignedInt();

			if(name == "barrel") {
				form.m_type = core::CategoryName::BARREL;
				this->Get<core::CategoryName::BARREL>().emplace_back(form);
			}
		}
	}
}

void TileMapParser::ParsePaths() {
	const auto group = m_tileMap->getObjectGroup("paths");
	if (group) {
		const auto& allObjects = group->getObjects();
		for (const auto& object : allObjects) {
			const auto& objMap = object.asValueMap();
			const auto type = objMap.at("type").asString();
			const auto name = objMap.at("name").asString();
			const auto x = objMap.at("x").asFloat();
			const auto y = objMap.at("y").asFloat();

			details::Form form;
			const auto& points { objMap.at("polylinePoints").asValueVector() };
			form.m_id = objMap.at("id").asUnsignedInt();
			form.m_points.reserve(points.size());
			for(const auto& pointValue: points) {
				const auto& pointMap = pointValue.asValueMap();
				form.m_points.emplace_back(pointMap.at("x").asFloat() + x, y - pointMap.at("y").asFloat());
			}
			form.m_type = core::CategoryName::PATH;
			this->Get<core::CategoryName::PATH>().emplace_back(form);
		}
	}
}

void TileMapParser::ParseInfluences() {
	const auto group = m_tileMap->getObjectGroup("influences");
	if (group) {
		const auto& allObjects = group->getObjects();
		for (const auto& object : allObjects) {
			const auto& objMap = object.asValueMap();
			const auto type = objMap.at("type").asString();
			const auto name = objMap.at("name").asString();
			const auto x = objMap.at("x").asFloat();
			const auto y = objMap.at("y").asFloat();

			details::Form form;
			form.m_points.emplace_back(x, y);
			form.m_id = objMap.at("id").asUnsignedInt();
			form.m_ownerId = objMap.at("owner-id").asUnsignedInt();
			form.m_type = core::CategoryName::INFLUENCE;
			cocos2d::Size size {
				objMap.at("width").asFloat(),
				objMap.at("height").asFloat()
			};
			form.m_rect = cocos2d::Rect{ form.m_points.front(), size };
			this->Get<core::CategoryName::INFLUENCE>().emplace_back(form);
		}
	}
}

void TileMapParser::Parse() {
    this->ParseUnits();
    this->ParseProps();
    this->ParsePaths();
    this->ParseInfluences();

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

		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				const auto tileGid = obstaclesLayer->getTileGIDAt({
					static_cast<float>(x),
					static_cast<float>(y) 
				});

				if(tileGid) {
					// { bodyType, categoryName}
					const auto [srcBodyType, srcCategoryName] = GetTileInfo(tileGid);
					
					if(srcBodyType == "static" && !isVisited[y][x]) {
						isVisited[y][x] = true;
						const auto category { core::CategoryFromString(srcCategoryName) };
						// Some bodies can consist from several tiles so
						// they should be combined into one physics body.

						// merge all horizontal neighbors:
						auto col { x + 1 };
						while(col < width && !isVisited[y][col]) {
							// if the right tile is same (static & categoory)
							// then merge them
							const auto gid = obstaclesLayer->getTileGIDAt({
								static_cast<float>(col),
								static_cast<float>(y) 
							});
							if(!gid) break;
							
							if(const auto [neighborBody, neighborCategory] = GetTileInfo(gid);
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
						form.m_type = category;

						this->Get(category).emplace_back(form);
					} 
				}	// tileGid
			}	// for
		}	// for
	}
}