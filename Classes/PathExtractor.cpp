#include "PathExtractor.hpp"

#include <iterator>     // std::distance
#include <algorithm>    // std::find
#include <cassert>
#include <limits>

#include "Bot.hpp"

using namespace path;

PathExtractor::PathExtractor(
    const path::PathSet * pathes, 
    const cocos2d::FastTMXTiledMap * tilemap
) : 
    m_tileSize{ tilemap->getTileSize().width },
    m_mapHeight{ tilemap->getMapSize().height },
    m_pathes { pathes }
{
    assert(m_tileSize > 0.f && m_mapHeight > 0.f && "Map isn't initialized at this point!");
}

Path PathExtractor::ExtractPathFor(const Enemies::Bot* bot) {
    const auto closestNodeIndex = this->FindClosestNodeIndex(bot->getPosition());

    std::vector<size_t> connectedNodes;
    connectedNodes.reserve(3);

    // go through all connected vertices and add them to `connectedNodes` vector
    // all these vertices are part of the unit's path
    std::vector<char> visited(m_pathes->waypoints.size(), false);
    this->Dfs(closestNodeIndex, connectedNodes, visited);

    Path path{};
    // copy waypoints with reflected Y coordinate to Path struct
    path.m_waypoints.resize(connectedNodes.size());
    size_t destinationIndex { 0 };
    for(auto sourceIndex: connectedNodes) {
        path.m_waypoints[destinationIndex] = m_tileSize * this->ReflectOrdinate(m_pathes->waypoints[sourceIndex]);
        path.m_waypoints[destinationIndex].x += m_tileSize / 2.f;
        ++destinationIndex;
    }
    
    // create new adjacency matrix for the given unit ID,
    // excluding information about pathes of other units 
    path.m_neighbours.resize(connectedNodes.size());    
    size_t pathNodeIndex { 0 };
    for(auto nodeIndex: connectedNodes) {
        const auto& neighbourList = m_pathes->adjacency[nodeIndex]; // vector<pair<size_t, Action>> 
        for(auto [originIndex, action]: neighbourList ) {
            const auto mappedIndex { this->GetIndexOf(originIndex, connectedNodes) };
            assert(mappedIndex != std::numeric_limits<size_t>::max() && "Index wasn't mapped!");
            path.m_neighbours[pathNodeIndex].emplace_back(mappedIndex, action);
        }
        ++pathNodeIndex;
    }

    return path;
}

void PathExtractor::Dfs(
    size_t index, 
    std::vector<size_t>& connectedNodes,
    std::vector<char>& visited
) const {
    if( visited[index] ) {
        return;
    }
    connectedNodes.emplace_back(index);
    visited[index] = true;
    for(auto [neighbour, action]: m_pathes->adjacency[index]) {
        if( !visited[neighbour] ) {
            this->Dfs(neighbour, connectedNodes, visited);
        }
    }
}

size_t PathExtractor::FindClosestNodeIndex(const cocos2d::Vec2& position) const noexcept {
    const auto coordinates { position / m_tileSize };
    size_t closest { std::numeric_limits<size_t>::max() };
    auto distance { std::numeric_limits<float>::max() };
    size_t index { 0 };
    for(const auto& point: m_pathes->waypoints) {
        const auto reflected = this->ReflectOrdinate(point);
        const auto dist { fabs(reflected.x - coordinates.x) + fabs(reflected.y - coordinates.y) };
        if(dist < distance) {
            distance = dist;
            closest = index;
        }
        ++index;
    } 
    return closest;
}

size_t PathExtractor::GetIndexOf(size_t value, const std::vector<size_t>& container) const noexcept {
    auto it = std::find(container.cbegin(), container.cend(), value);
    size_t index { std::numeric_limits<size_t>::max() };
    if(it != container.cend()) {
        index = static_cast<size_t>(std::distance(container.cbegin(), it));
    }
    return index;
};