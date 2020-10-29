#include "TileMapParser.hpp"
#include "cocos2d.h"

#include "Utils.hpp"
#include "TileMapHelper.hpp"
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
		else if ( ty == core::EntityNames::SLIME ) {
			type = core::EnemyClass::SLIME;
		}
		else if ( ty == core::EntityNames::SPIDER ) {
			type = core::EnemyClass::SPIDER;
		}
		else if ( ty == core::EntityNames::BOULDER_PUSHER ) {
			type = core::EnemyClass::BOULDER_PUSHER;
		}
		// assert(type == core::EnemyClass::UNDEFINED && "[TileMapParser] Enemy class not defined!");
		return type;
	}

	// This thing is used only by TileMapParser::Parse for merging bodies
	core::CategoryName CategoryFromProperty(size_t mask) noexcept {
		using Property = TileMap::Property;

		auto property = Utils::EnumCast<Property>(mask);
		auto category = core::CategoryName::UNDEFINED;
		switch(property) {
			case Property::BORDER: category = core::CategoryName::BORDER; break;
			case Property::SPIKE: category = core::CategoryName::SPIKES; break;
			case Property::PLATFORM: category = core::CategoryName::PLATFORM; break;
			case Property::EMPTY: case Property::SOLID: case Property::OUTSIDE_MAP: case Property::UNDEFINED: break;
			default: assert(false && "unexpected property"); break;
		}

		return category;
	} 
}

// simulate behaviour
class Move {
public:
	Move(cocos2d::Vec2 start, int w, int h) : 
		m_position { start },
		m_width { w }, 
		m_height { h } 
	{}

	void TurnRight() noexcept {
		m_shiftIndex = (m_shiftIndex + 1) % 4;
	}
	
	bool CanMoveForward() const noexcept {
		return this->WithinBounds(m_position + m_shift[m_shiftIndex]);
	}

	void MakeMoveForward() noexcept {
		m_position += m_shift[m_shiftIndex];
	}

	void MakeMoveBackwards() noexcept {
		m_position -= m_shift[m_shiftIndex];
	}

	cocos2d::Vec2 GetPosition() const noexcept {
		return m_position;
	}

	void Reset(const cocos2d::Vec2& pos) noexcept {
		m_position = pos;
		m_shiftIndex = 0;
	}

	cocos2d::Vec2 PeekNext() const noexcept {
		return m_position + m_shift[m_shiftIndex];
	}

private:
	bool WithinBounds(const cocos2d::Vec2 & pos) const noexcept {
		return pos.x >= 0.f 
			&& pos.y >= 0.f 
			&& pos.x < static_cast<float>(m_width) 
			&& pos.y < static_cast<float>(m_height); 
	}

	inline static const cocos2d::Vec2 m_shift[4] = { 
		{ 1.f, 0.f }, 
		{ 0.f, 1.f }, 
		{ -1.f, 0.f }, 
		{ 0.f, -1.f } 
	};
	
	cocos2d::Vec2 m_position {};
	
	size_t m_shiftIndex { 0 };

	const int m_width { 0 };
	const int m_height { 0 };
};

class BorderBuilder {
public:
	using Property = TileMap::Property;

	BorderBuilder(const TileMap::Cache * cache)
		: m_cache { cache }
	{
		assert(m_cache && "cache pointer can't be null");
		assert(m_cache->blocksLayer && "can't find the layer with blocks");
	}

	std::vector<std::list<cocos2d::Vec2>> BuildBorder() const {
		// BUILD BORDERS FOR COMPOSITE PHYSICS BODIES:
		int components { 0 };
		std::vector<std::vector<char>> isVisited (
			m_cache->mapHeight, 
			std::vector<char>(m_cache->mapWidth, false)
		);
		std::vector<std::list<cocos2d::Vec2>> tiles;
		for(size_t y = 0; y < m_cache->mapHeight; y++) {
			for(size_t x = 0; x < m_cache->mapWidth; x++) {
				if(isVisited[y][x]) continue;
				// always top-left tile
				cocos2d::Vec2 point { static_cast<float>(x), static_cast<float>(y) };
				auto mask = m_cache->GetProperties(point);

				// skip not a border
				if(!Utils::HasAny(mask, Property::BORDER)) continue;
				
				components++;
				tiles.emplace_back();
				tiles.back().emplace_back(point);

				// go in one direction from the found border tile
				this->Visit(point, [&tiles = tiles.back()](const cocos2d::Vec2& tile) {
					tiles.push_back(tile);
				}, isVisited);
				// reset start m_position
				point = cocos2d::Vec2{ static_cast<float>(x), static_cast<float>(y) };
				// try to go in another direction from the found border tile
				this->Visit(point, [&tiles = tiles.back()](const cocos2d::Vec2& tile) {
					tiles.push_front(tile);
				}, isVisited);
			}
		}
		
		return tiles;
	}

	std::vector<details::Form> GetChainedLines(const std::vector<std::list<cocos2d::Vec2>>& tilesBorder) const {
		std::vector<details::Form> forms;
		forms.reserve(tilesBorder.size());

		for(auto&& chain: tilesBorder) {
			details::Form form;
			form.m_type = core::CategoryName::BORDER;
			form.m_points.reserve(chain.size() >> 1U);
			for(auto& point: chain) {
				auto result = this->AddPoint(point);
				if(result.has_value()) {
					form.m_points.emplace_back(*result);
				}
			}
			assert(chain.size() >= 2 && "Can't have a chain with 0 or 1 elements");
			// if distance between front and back tiles is short enough => connect them
			float dx = chain.front().x - chain.back().x;
			float dy = chain.front().y - chain.back().y;
			auto distance = fabs(dx) + fabs(dy);
			bool isSameLine = helper::IsEqual(dx, 0.f, 0.1f) || helper::IsEqual(dy, 0.f, 0.1f);
			if(distance <= 10.f && isSameLine) {
				form.m_points.emplace_back(form.m_points.front());
			}
			forms.emplace_back(form);
		}

		return forms;
	}

private:

	std::optional<cocos2d::Vec2> AddPoint(const cocos2d::Vec2& point) const {
		// check for { empty tiles | map boundary | (not solid && not border ) } around the point
		cocos2d::Vec2 shiftsToFree[4] = {};
		cocos2d::Vec2 shiftsToBorder[4] = {};
		int borderTileCount { 0 };
		// tiles that aren't occupied by border (but include out of map bounds points)
		int freeTileCount { 0 };
		int countOutsideMap { 0 };

		for(size_t i = 0; i < 4; ++i) {
			const auto neighbor = point + m_horizontalDelta[i];
			const auto mask = m_cache->GetProperties(neighbor);
			if(Utils::HasAny(mask, Property::OUTSIDE_MAP)) {
				shiftsToFree[freeTileCount] = m_horizontalDelta[i];
				++freeTileCount;
				++countOutsideMap;
			}
			else if(!Utils::HasAny(mask, Property::BORDER, Property::SOLID)) {
				shiftsToFree[freeTileCount] = m_horizontalDelta[i];
				++freeTileCount;
			} 
			else if(Utils::HasAny(mask, Property::BORDER)) {
				shiftsToBorder[borderTileCount] = m_horizontalDelta[i];
				++borderTileCount;
			}
		}
		assert(freeTileCount >= 0 && freeTileCount <= 2 && "Can't have more than 2 neighbours");
		cocos2d::Vec2 freeSum { 
			shiftsToFree[0].x + shiftsToFree[1].x, 
			shiftsToFree[0].y + shiftsToFree[1].y 
		};
		cocos2d::Vec2 borderSum { 
			shiftsToBorder[0].x + shiftsToBorder[1].x, 
			shiftsToBorder[0].y + shiftsToBorder[1].y 
		};

		// One of cardinal sequence: N->E; E->S; S->W; W->N is a pair of neighbours
		bool hasFreeTilesCardinalContinuation = helper::IsEqual(fabs(freeSum.x) + fabs(freeSum.y), 2.f, 0.01f);
		bool hasFreeTilesInMap = freeTileCount != countOutsideMap;
		bool hasBorderTilesCardinalContinuation = !helper::IsEqual(fabs(borderSum.x) + fabs(borderSum.y), 0.f, 0.1f);

		if(freeTileCount == 2 && hasFreeTilesCardinalContinuation) {
			// From tile coordinates in Tiled.exe to coordinates in game engine
			cocos2d::Vec2 tileMiddle { 
				point.x * m_cache->tileSize.width + m_cache->tileSize.width / 2.f, 
				(m_cache->mapHeight - point.y - 1.f) * m_cache->tileSize.height + m_cache->tileSize.height / 2.f
			};
			tileMiddle.x +=  freeSum.x * m_cache->tileSize.width  / 2.f;
			tileMiddle.y += -freeSum.y * m_cache->tileSize.height / 2.f;
			return { tileMiddle };
		}
		else if(!hasFreeTilesInMap && hasBorderTilesCardinalContinuation) { 
			// this is a case for some corner tiles (they indicates a concave polygons)
			// or tiles on the map's border.
			// From tile coordinates in Tiled.exe to coordinates in game engine
			cocos2d::Vec2 tileMiddle { 
				point.x * m_cache->tileSize.width + m_cache->tileSize.width / 2.f, 
				(m_cache->mapHeight - point.y - 1.f) * m_cache->tileSize.height + m_cache->tileSize.height / 2.f
			};
		
			for(int i = 0; i < 4; ++i) {
				const auto neighbor = point + m_diagonalDelta[i];
				const auto mask = m_cache->GetProperties(neighbor);
				if (!Utils::HasAny(mask, Property::OUTSIDE_MAP, Property::BORDER, Property::SOLID)) {
					tileMiddle.x +=  m_diagonalDelta[i].x * m_cache->tileSize.width  / 2.f;
					tileMiddle.y += -m_diagonalDelta[i].y * m_cache->tileSize.height / 2.f;
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
	 * @param start is a m_position of the tile on tile map from which the algo starts
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
			static_cast<int>(m_cache->mapWidth), 
			static_cast<int>(m_cache->mapHeight)
		};
		isVisited[static_cast<size_t>(start.y)][static_cast<size_t>(start.x)] = true;
		
		auto point = start;

		for(size_t turns = 0, skips = 0; ; turns++) {
			size_t steps { 0 };
			// trying to move
			while(move.CanMoveForward()) {
				// move 1 tile forward
				move.MakeMoveForward();
				// update m_position
				point = move.GetPosition();
				// get tile gid
				size_t mask = m_cache->GetProperties(point);
				const auto x = static_cast<size_t>(point.x);
				const auto y = static_cast<size_t>(point.y);
				// add tile if it's not visited yet and is border tile
				if(!isVisited[y][x] && Utils::HasAny(mask, Property::BORDER)) {
					// mark
					isVisited[y][x] = true;
					add(point);
					steps++;
				}
				else {
					// restore m_position
					move.MakeMoveBackwards();
					break;
				}
			}
			
			skips = steps? 0: skips + 1;
			if(skips >= 4) break;
			move.TurnRight();
		}
	}

private:
	// Info about the map that need to be extracted only once
	// to speed up and make things easier accessable 
	const TileMap::Cache * m_cache { nullptr };

	static inline const cocos2d::Vec2 m_horizontalDelta[] = {
		{-1.f, 0.f}, {1.f, 0.f}, {0.f, 1.f}, {0.f, -1.f}
	};

	static inline const cocos2d::Vec2 m_diagonalDelta[4] = {
		{-1.f, -1.f}, {-1.f, 1.f}, {1.f, 1.f}, {1.f, -1.f}
	};
};

TileMapParser::TileMapParser(const cocos2d::FastTMXTiledMap * tileMap)
	: m_tileMap{ tileMap }
	, m_tileMapCache{ std::make_unique<TileMap::Cache>(m_tileMap) }
{
    this->Get<CategoryName::PLATFORM>().reserve(20);
	this->Get<CategoryName::BORDER>().reserve(100);
	this->Get<CategoryName::BARREL>().reserve(10);
	this->Get<CategoryName::ENEMY>().reserve(30);
	this->Get<CategoryName::PATH>().reserve(30);
	this->Get<CategoryName::INFLUENCE>().reserve(30);
	this->Get<CategoryName::SPIKES>().reserve(10);
}

TileMapParser::~TileMapParser() = default;

void TileMapParser::Parse() {
    this->ParseUnits();
    this->ParseProps();
    this->ParsePaths();
    this->ParseInfluences();

	// parse borders:
	BorderBuilder builder{ m_tileMapCache.get() };
	auto tilesBorder = builder.BuildBorder();
	this->Get<CategoryName::BORDER>() = builder.GetChainedLines(tilesBorder);

    const auto obstaclesLayer = m_tileMap->getLayer(TileMap::COLLISION_LAYER_NAME);
	if (obstaclesLayer) {
		const auto tileSize { obstaclesLayer->getMapTileSize() };
		const auto mapSize { obstaclesLayer->getLayerSize() };
		const auto width { static_cast<int>(mapSize.width) };
		const auto height { static_cast<int>(mapSize.height) };

		// check whether the tile was already visited
		// for now I don't need it.
    	std::vector<std::vector<char>> isVisited (height, std::vector<char>(width, false));
		
		auto GetTileInfo = [this](int tileGid) {
			const auto tileProp { m_tileMap->getPropertiesForGID(tileGid) };
			const auto& properties { tileProp.asValueMap() };
			const auto categoryNameIter { properties.find("category-name") };
			
			assert(categoryNameIter != properties.end() && "Can't find body type or category");
			return categoryNameIter->second.asString();
		};
		
		// merge tiles with similar properties
		using Property = TileMap::Property;
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				auto originProperty = m_tileMapCache->GetProperties(y, x);
				
				if(Utils::HasAny(originProperty, Property::EMPTY, Property::BORDER) || isVisited[y][x]) continue;

				isVisited[y][x] = true;
				// Some bodies can consist from several tiles so
				// they should be combined into one physics body.
				// merge all horizontal neighbors:
				auto col { x + 1 };
				while(col < width && !isVisited[y][col]) {
					// if they have same property then merge them
					auto neighbourProperty = m_tileMapCache->GetProperties(y, col);
					if(neighbourProperty != originProperty) {
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
				form.m_type = CategoryFromProperty(originProperty);
				this->Get(form.m_type).emplace_back(form);
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
				form.m_type = CategoryName::PLAYER;
				this->Get<CategoryName::PLAYER>().emplace_back(form);
			}
			else if(name == "enemy") {
				form.m_type = CategoryName::ENEMY;
				form.m_pathId = objMap.at("path-id").asUnsignedInt();
				form.m_enemyClass = ::AsEnemyClass(type);
				this->Get<CategoryName::ENEMY>().emplace_back(form);
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
				form.m_type = CategoryName::BARREL;
				this->Get<CategoryName::BARREL>().emplace_back(form);
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
			form.m_type = CategoryName::PATH;
			this->Get<CategoryName::PATH>().emplace_back(form);
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
			form.m_type = CategoryName::INFLUENCE;
			cocos2d::Size size {
				objMap.at("width").asFloat(),
				objMap.at("height").asFloat()
			};
			form.m_rect = cocos2d::Rect{ form.m_points.front(), size };
			this->Get<CategoryName::INFLUENCE>().emplace_back(form);
		}
	}
}
