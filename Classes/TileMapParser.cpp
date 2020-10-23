#include "TileMapParser.hpp"
#include "cocos2d.h"

#include "PhysicsHelper.hpp"
#include <list>
#include <optional>
#include <cassert>

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
		else if ( ty == core::EntityNames::BOULDER_PUSHER ) {
			type = core::EnemyClass::BOULDER_PUSHER;
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

// simulate behaviour
struct Move {

	Move(cocos2d::Vec2 start, int w, int h) : 
		position { start },
		width { w }, 
		height { h } 
	{}

	void TurnRight() noexcept {
		shiftIndex = (shiftIndex + 1) % 4;
	}
	
	bool CanMoveForward() const noexcept {
		return this->WithinBounds(position + shift[shiftIndex]);
	}

	void MakeMoveForward() noexcept {
		position += shift[shiftIndex];
	}

	void MakeMoveBackwards() noexcept {
		position -= shift[shiftIndex];
	}

	cocos2d::Vec2 GetPosition() const noexcept {
		return position;
	}

	void Reset(const cocos2d::Vec2& pos) noexcept {
		position = pos;
		shiftIndex = 0;
	}

	cocos2d::Vec2 PeekNext() const noexcept {
		return position + shift[shiftIndex];
	}

private:
	bool WithinBounds(const cocos2d::Vec2 & pos) const noexcept {
		return pos.x >= 0.f 
			&& pos.y >= 0.f 
			&& pos.x < static_cast<float>(width) 
			&& pos.y < static_cast<float>(height); 
	}

	inline static const cocos2d::Vec2 shift[4] = { 
		{ 1.f, 0.f }, 
		{ 0.f, 1.f }, 
		{ -1.f, 0.f }, 
		{ 0.f, -1.f } 
	};
	
	cocos2d::Vec2 position {};
	
	size_t shiftIndex { 0 };

	const int width { 0 };
	const int height { 0 };
};

void TileMapParser::Parse() {
    this->ParseUnits();
    this->ParseProps();
    this->ParsePaths();
    this->ParseInfluences();

    const auto obstaclesLayer = m_tileMap->getLayer("padding-background");
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

				if (exist) {
					info = std::make_pair(bodyTypeIter->second.asString(), categoryNameIter->second.asString());
				}
			}

			return info;
		};
		
		auto IsBorder = [this](int gid) {
			if (gid) {
				const auto tileProp { m_tileMap->getPropertiesForGID(gid) };
				const auto& properties = tileProp.asValueMap();
				const auto categoryNameIter = properties.find("category-name");
				const auto exist { categoryNameIter != properties.end() };
				if (exist) {
					return categoryNameIter->second.asString() == "border";
				}
			}
			return false;
		};

		auto InMap = [width, height](const cocos2d::Vec2 & pos) {
			return pos.x >= 0.f 
				&& pos.y >= 0.f 
				&& pos.x < static_cast<float>(width) 
				&& pos.y < static_cast<float>(height); 
		};

		auto IsFree = [this](int gid) {
			if (gid) {
				const auto tileProp { m_tileMap->getPropertiesForGID(gid) };
				const auto& properties = tileProp.asValueMap();

				const auto categoryNameIter = properties.find("category-name");
				const auto exist { categoryNameIter != properties.end() };
				if (!exist) return true;
				auto name = categoryNameIter->second.asString();
				return (name != "border" && name != "solid");
			}
			// empty =>
			return true;
		};

		auto AddPoint = [
			this
			, width
			, height
			, tileSize
			, IsFree
			, InMap
			, IsBorder
			, &obstaclesLayer
		](const cocos2d::Vec2 & point) -> std::optional<cocos2d::Vec2> {
			// check for { empty tiles | map boundary | (not solid && not border ) } around the point
			cocos2d::Vec2 shifts[4] = {};
			int count { 0 };
			int countOutsideMap { 0 };
			float dx[] = {-1.f, 1.f, 0.f,  0.f};
			float dy[] = { 0.f, 0.f, 1.f, -1.f};
			for(int i = 0; i < 4; ++i) {
				auto neighbor = point + cocos2d::Vec2{ dx[i], dy[i] };
				if(!InMap(neighbor)) {
					shifts[count] = { dx[i], dy[i] };
					++count;
					++countOutsideMap;
				}
				else if(auto gid = obstaclesLayer->getTileGIDAt(neighbor); IsFree(gid)) {
					shifts[count] = { dx[i], dy[i] };
					++count;
				}
			}
			assert(count >= 0 && count <= 2 && "Can't have more than 2 neighbours");
			cocos2d::Vec2 sum { shifts[0].x + shifts[1].x, shifts[0].y + shifts[1].y };
			if(count == 2 && helper::IsEquel(fabs(sum.x) + fabs(sum.y), 2.f, 0.01f)) {
				// { -1, 0 } --> { 0,  1 }
				// { 0,  1 } --> { 1,  0 }
				// { 1,  0 } --> { 0  -1 }
				// { 0, -1 } --> { -1  0 }
				// =======================
				cocos2d::Vec2 tileMiddle { 
					point.x * tileSize.width + tileSize.width / 2.f, 
					(height - point.y - 1.f) * tileSize.height + tileSize.height / 2.f
				};
				tileMiddle.x +=  sum.x * tileSize.width  / 2.f;
				tileMiddle.y += -sum.y * tileSize.height / 2.f;
				return { tileMiddle };
			}
			else if(count - countOutsideMap == 0) { // this is a case for some corner tiles (they indicates a concave polygons)
				cocos2d::Vec2 tileMiddle { 
					point.x * tileSize.width + tileSize.width / 2.f, 
					(height - point.y - 1.f) * tileSize.height + tileSize.height / 2.f
				};
				cocos2d::Vec2 shift[4] = {
					{-1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}, {1.f, -1.f}
				};
				for(int i = 0; i < 4; ++i) {
					auto neighbor = point + shift[i];
					if(InMap(neighbor)) {
						auto gid = obstaclesLayer->getTileGIDAt(neighbor);
						if(IsFree(gid)) {
							tileMiddle.x +=  shift[i].x * tileSize.width  / 2.f;
							tileMiddle.y += -shift[i].y * tileSize.height / 2.f;
							break;
						}
					}
				}
				return { tileMiddle };
			}
			else {
				return std::nullopt;
			}
		};

		// BUILD BORDERS FOR COMPOSITE PHYSICS BODIES:
		int components { 0 };
		std::vector<std::list<cocos2d::Vec2>> tiles;
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				if(isVisited[y][x]) continue;
				// always top-left tile
				cocos2d::Vec2 point { static_cast<float>(x), static_cast<float>(y) };
				auto tileGid = obstaclesLayer->getTileGIDAt(point);

				if(!IsBorder(tileGid)) continue;
				
				components++;
				tiles.emplace_back();

				Move move { point, width, height };
				tiles[components - 1].emplace_back(point);
				isVisited[(int)point.y][(int)point.x] = true;
				
				for(int turns = 0, skips = 0; ; turns++) {
					int steps { 0 };
					// trying to move
					while(move.CanMoveForward()) {
						// move 1 tile forward
						move.MakeMoveForward();
						// update position
						point = move.GetPosition();
						// get tile gid
						tileGid = obstaclesLayer->getTileGIDAt(point);
						// add tile if it's not visited yet and is border tile
						if(!isVisited[(int)point.y][(int)point.x] && IsBorder(tileGid)) {
							// mark
							isVisited[(int)point.y][(int)point.x] = true;
							tiles[components - 1].emplace_back(point);
							steps++;
						}
						else {
							// restore position
							move.MakeMoveBackwards();
							break;
						}
					}
					
					skips = steps? 0: skips + 1;
					if(skips >= 4) break;
					move.TurnRight();
				}

				move.Reset({(float)x, (float)y});
				for(int turns = 0, skips = 0; ; turns++) {
					int steps { 0 };
					// trying to move
					while(move.CanMoveForward()) {
						// move 1 tile forward
						move.MakeMoveForward();
						// update position
						point = move.GetPosition();
						// get tile gid
						tileGid = obstaclesLayer->getTileGIDAt(point);
						// add tile if it's not visited yet and is border tile
						if(!isVisited[(int)point.y][(int)point.x] && IsBorder(tileGid)) {
							// mark
							isVisited[(int)point.y][(int)point.x] = true;
							tiles[components - 1].emplace_front(point);
							steps++;
						}
						else {
							// restore position
							move.MakeMoveBackwards();
							break;
						}
					}
					
					skips = steps? 0: skips + 1;
					if(skips >= 4) break;
					move.TurnRight();
				}
			}
		}
		
		// CREATE CHAINED LINES
		for(auto&& chain: tiles) {
			details::Form form;
			form.m_type = core::CategoryName::BORDER;
			form.m_points.reserve(chain.size() >> 1U);
			for(auto& point: chain) {
				auto result = AddPoint(point);
				if(result.has_value()) {
					form.m_points.emplace_back(*result);
				}
			}
			this->Get(form.m_type).emplace_back(form);
		}


		for(auto& rows: isVisited) {
			for(auto& v: rows) {
				v = false;
			}
		}

		// END OF THE SHITTY TEST CODE

		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				const auto tileGid = obstaclesLayer->getTileGIDAt({
					static_cast<float>(x),
					static_cast<float>(y) 
				});
				if(!tileGid) continue;
				// { bodyType, categoryName}
				const auto [srcBodyType, srcCategoryName] = GetTileInfo(tileGid);
				
				if(srcBodyType == "static" && !isVisited[y][x]) {
					isVisited[y][x] = true;
					const auto category { core::CategoryFromString(srcCategoryName) };
					if(category == core::CategoryName::BORDER) continue;
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
			}	// for
		}	// for
	}
}