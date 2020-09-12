#include "PathNodes.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

#include "cocos2d.h"

#include <string>
#include <cstring>

rapidjson::Document path::PathSet::Load(size_t id) {
    rapidjson::Document doc;

    const auto filepath { cocos2d::StringUtils::format(pathTemplate, id) };

    const auto fileUtils = cocos2d::FileUtils::getInstance();
    const auto data = fileUtils->getDataFromFile(filepath);

    if(!data.isNull()){
        std::string str(reinterpret_cast<char *>(data.getBytes()), data.getSize());
        (void) doc.Parse(str.c_str());
        if(doc.HasParseError()) {
            /// TODO: fail to parse json
        }
    } 
    else {
        ///TODO: fail to find file
    }
    return doc;
}

void path::PathSet::Parse(rapidjson::Document& doc) {
    waypoints.reserve(100);
    const auto& points = doc["waypoints"].GetArray(); 
    for(const auto& p: points) {
        waypoints.emplace_back(
            static_cast<float>(p[0].GetInt()), 
            static_cast<float>(p[1].GetInt())
        );
    }

    adjacency.resize(waypoints.size());
    size_t vertexIndex { 0 };
    const auto& trees = doc["trees"].GetArray();
    for(const auto& tree: trees) {
        const auto neighbours { tree.GetArray() };
        for(const auto& neighbour: neighbours) {
            const auto action { !strcmp(neighbour["action"].GetString(), "move")? Action::MOVE: Action::JUMP };
            adjacency[vertexIndex].emplace_back( neighbour["vert"].GetInt(), action );
        }
        vertexIndex++;
    }
}