#include "TileMapParser.hpp"
#include "cocos2d.h"

#include "Utils.hpp"
#include "PhysicsHelper.hpp"
#include <list>
#include <optional>
#include <functional>
#include <iterator>
#include <cassert>
#include <cstdint>

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

struct BorderBuilder {
	enum Category : uint8_t {
		BORDER 		= 0x1,
		EMPTY  		= 0x2, // no tile in placeholder
		OUTSIDE_MAP = 0x4,
		SOLID 		= 0x8,
		SPIKE 		= 0x10,
		PLATFORM 	= 0x20,
		UNDEFINED 	= 0x40
	};
	using Mask = uint8_t;

	BorderBuilder(const cocos2d::FastTMXTiledMap * tilemap)
		: m_tileMap { tilemap }
	{
		// initialize the cache
		m_cache.blocksLayer = m_tileMap->getLayer(LAYER_NAME);
		assert(m_cache.blocksLayer && "can't find the layer with blocks");
		m_cache.tileSize = m_cache.blocksLayer->getMapTileSize();
		const auto mapSize { m_cache.blocksLayer->getLayerSize() };
		m_cache.mapHeight = static_cast<int>(mapSize.height);
		m_cache.mapWidth = static_cast<int>(mapSize.width);
		m_cache.isCached = true;
	}

	std::optional<cocos2d::Vec2> AddPoint(const cocos2d::Vec2& point) const {
		// check for { empty tiles | map boundary | (not solid && not border ) } around the point
		cocos2d::Vec2 shifts[4] = {};
		cocos2d::Vec2 shiftsToBorder[4] = {};
		int borderTileCount {0};
		int count { 0 };
		int countOutsideMap { 0 };
		const float dx[] = {-1.f, 1.f, 0.f,  0.f};
		const float dy[] = { 0.f, 0.f, 1.f, -1.f};

		for(size_t i = 0; i < 4; ++i) {
			const auto neighbor = point + cocos2d::Vec2{ dx[i], dy[i] };
			const auto mask = this->GetProperties(neighbor);
			if((mask & Category::OUTSIDE_MAP) > 0) {
				shifts[count] = { dx[i], dy[i] };
				++count;
				++countOutsideMap;
			}
			else if((mask & Utils::CreateMask(Category::BORDER, Category::SOLID)) == 0) {
				shifts[count] = { dx[i], dy[i] };
				++count;
			} 
			else if((mask & Category::BORDER) > 0) {
				shiftsToBorder[borderTileCount] = { dx[i], dy[i] };
				++borderTileCount;
			}
		}
		assert(count >= 0 && count <= 2 && "Can't have more than 2 neighbours");
		cocos2d::Vec2 sum { shifts[0].x + shifts[1].x, shifts[0].y + shifts[1].y };
		cocos2d::Vec2 borderSum { shiftsToBorder[0].x + shiftsToBorder[1].x, shiftsToBorder[0].y + shiftsToBorder[1].y };

		if(count == 2 && helper::IsEquel(fabs(sum.x) + fabs(sum.y), 2.f, 0.01f)) {
			// { -1, 0 } --> { 0,  1 }
			// { 0,  1 } --> { 1,  0 }
			// { 1,  0 } --> { 0  -1 }
			// { 0, -1 } --> { -1  0 }
			// =======================
			cocos2d::Vec2 tileMiddle { 
				point.x * m_cache.tileSize.width + m_cache.tileSize.width / 2.f, 
				(m_cache.mapHeight - point.y - 1.f) * m_cache.tileSize.height + m_cache.tileSize.height / 2.f
			};
			tileMiddle.x +=  sum.x * m_cache.tileSize.width  / 2.f;
			tileMiddle.y += -sum.y * m_cache.tileSize.height / 2.f;
			return { tileMiddle };
		}
		else if(count - countOutsideMap == 0 && !helper::IsEquel(fabs(borderSum.x) + fabs(borderSum.y), 0.f, 0.1f)) { 
			// this is a case for some corner tiles (they indicates a concave polygons)
			// or tiles new the map's border
			cocos2d::Vec2 tileMiddle { 
				point.x * m_cache.tileSize.width + m_cache.tileSize.width / 2.f, 
				(m_cache.mapHeight - point.y - 1.f) * m_cache.tileSize.height + m_cache.tileSize.height / 2.f
			};
			
			cocos2d::Vec2 diagonalShift[4] = {
				{-1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}, {1.f, -1.f}
			};
			for(int i = 0; i < 4; ++i) {
				const auto neighbor = point + diagonalShift[i];
				const auto mask = this->GetProperties(neighbor);
				if ((mask & Category::OUTSIDE_MAP) == 0 && 
					(mask & Utils::CreateMask(Category::BORDER, Category::SOLID)) == 0
				) {
					tileMiddle.x +=  diagonalShift[i].x * m_cache.tileSize.width  / 2.f;
					tileMiddle.y += -diagonalShift[i].y * m_cache.tileSize.height / 2.f;
					break;
				}
			}
			return { tileMiddle };
		}
		else {
			return std::nullopt;
		}
	}

	/**
	 * Go from the start through the first met neighbour to extract a border of physics object
	 * 
	 * @param start is a position of the tile on tile map from which the algo starts
	 * @param tiles is a container for the border tiles
	 * @param adder is a functional object which define where the tile will be added
	 * @param isVisited is a 2d map of visited tiles
	 */
	void Visit(
		const cocos2d::Vec2& start
		, std::function<void(const cocos2d::Vec2&)> && add
		, std::vector<std::vector<char>>& isVisited
	) const {
		Move move { start, 
			static_cast<int>(m_cache.mapWidth), 
			static_cast<int>(m_cache.mapHeight)
		};
		isVisited[static_cast<size_t>(start.y)][static_cast<size_t>(start.x)] = true;
		
		auto point = start;

		for(size_t turns = 0, skips = 0; ; turns++) {
			size_t steps { 0 };
			// trying to move
			while(move.CanMoveForward()) {
				// move 1 tile forward
				move.MakeMoveForward();
				// update position
				point = move.GetPosition();
				// get tile gid
				size_t mask = this->GetProperties(point);
				const auto x = static_cast<size_t>(point.x);
				const auto y = static_cast<size_t>(point.y);
				// add tile if it's not visited yet and is border tile
				if(!isVisited[y][x] && (mask & Category::BORDER) > 0) {
					// mark
					isVisited[y][x] = true;
					add(point);
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

	std::vector<std::list<cocos2d::Vec2>> BuildBorder() const {
		// BUILD BORDERS FOR COMPOSITE PHYSICS BODIES:
		int components { 0 };
		std::vector<std::vector<char>> isVisited (
			m_cache.mapHeight, 
			std::vector<char>(m_cache.mapWidth, false)
		);
		std::vector<std::list<cocos2d::Vec2>> tiles;
		for(size_t y = 0; y < m_cache.mapHeight; y++) {
			for(size_t x = 0; x < m_cache.mapWidth; x++) {
				if(isVisited[y][x]) continue;
				// always top-left tile
				cocos2d::Vec2 point { static_cast<float>(x), static_cast<float>(y) };
				auto mask = this->GetProperties(point);

				// skip not a border
				if((mask & Category::BORDER) == 0) continue;
				
				components++;
				tiles.emplace_back();
				tiles.back().emplace_back(point);

				// go in one direction from the found border tile
				this->Visit(point, [&tiles = tiles.back()](const cocos2d::Vec2& tile) {
					tiles.push_back(tile);
				}, isVisited);
				// reset start position
				point = cocos2d::Vec2{ static_cast<float>(x), static_cast<float>(y) };
				// try to go in another direction from the found border tile
				this->Visit(point, [&tiles = tiles.back()](const cocos2d::Vec2& tile) {
					tiles.push_front(tile);
				}, isVisited);
			}
		}
		
		return tiles;
	}

private:

	Mask GetProperties(const cocos2d::Vec2& pos) const noexcept {
		Mask mask { 0U };
		if(!IsInMap(pos)) {
			mask = Category::OUTSIDE_MAP;
			return mask;
		}
		assert(m_cache.isCached && "cache isn't initialized");
		const auto tileGid = m_cache.blocksLayer->getTileGIDAt(pos);
		if(!tileGid) {
			mask = Category::EMPTY;
			return mask;
		}
		const auto tileProp { m_tileMap->getPropertiesForGID(tileGid) };
		const auto& properties = tileProp.asValueMap();

		// const auto bodyTypeIter = properties.find("body-type");
		const auto categoryNameIter = properties.find("category-name");
		assert(categoryNameIter != properties.end() && "Can't find property: <category-name>" );

		auto name = categoryNameIter->second.asString();
		if(name == "border") {
			mask = Category::BORDER;
		}
		else if(name == "solid") {
			mask = Category::SOLID;
		}
		else if(name == "spikes") {
			mask = Category::SPIKE;
		}
		else if(name == "platform") {
			mask = Category::PLATFORM;
		}
		else {
			mask = Category::UNDEFINED;
		}
		return mask;
	}

	bool IsInMap(const cocos2d::Vec2& pos) const noexcept {
		return pos.x >= 0.f 
			&& pos.y >= 0.f 
			&& pos.x < static_cast<float>(m_cache.mapWidth) 
			&& pos.y < static_cast<float>(m_cache.mapHeight); 
	}

private:
	// Info about the map that need to be extracted only once
	// to speed up and make things easier accessable 
	struct MapInfo {
		size_t mapWidth { 0U };
		size_t mapHeight { 0U };
		cocos2d::Size tileSize { 0.f, 0.f }; // it's square
		cocos2d::FastTMXLayer * blocksLayer { nullptr };
		// indicates whether the values were already initialized or not
		bool isCached { false };
	};

	const cocos2d::FastTMXTiledMap * const m_tileMap { nullptr };

	// often accessable cached info extracted from the tilemap
	MapInfo m_cache;
	// name of layer which contains collidable tiles (border)
	inline static const char* LAYER_NAME = "padding-background";
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

				assert( 
					bodyTypeIter != properties.end() 
					&& categoryNameIter != properties.end() 
					&& "Can't find body type or category" 
				);

				info = std::make_pair(bodyTypeIter->second.asString(), categoryNameIter->second.asString());
			}

			return info;
		};
		
		BorderBuilder builder{ m_tileMap };
		auto tilesBorder = builder.BuildBorder();
		
		// CREATE CHAINED LINES
		for(auto&& chain: tilesBorder) {
			details::Form form;
			form.m_type = core::CategoryName::BORDER;
			form.m_points.reserve(chain.size() >> 1U);
			for(auto& point: chain) {
				auto result = builder.AddPoint(point);
				if(result.has_value()) {
					form.m_points.emplace_back(*result);
				}
			}
			assert(chain.size() >= 2 && "Can't have a chain with 0 or 1 elements");
			// if distance between front and back tiles is short enough => connect them
			float dx = chain.front().x - chain.back().x;
			float dy = chain.front().y - chain.back().y;
			auto distance = fabs(dx) + fabs(dy);
			bool isSameLine = helper::IsEquel(dx, 0.f, 0.1f) || helper::IsEquel(dy, 0.f, 0.1f);
			if(distance <= 10.f && isSameLine) {
				form.m_points.emplace_back(form.m_points.front());
			}
			this->Get(form.m_type).emplace_back(form);
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
