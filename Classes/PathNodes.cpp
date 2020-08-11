#include "PathNodes.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

#include "cocos2d.h"

#include <string>
#include <cstring>

rapidjson::Document path::Supplement::Load(size_t id) {
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
    } else {
        ///TODO: fail to find file
    }
    return doc;
}

void path::Supplement::Parse(rapidjson::Document& doc) {
    waypoints.reserve(100);
    const auto& points = doc["waypoints"].GetArray(); 
    for(const auto& p: points) {
        waypoints.emplace_back(p[0].GetInt(), p[1].GetInt());
    }

    adj.resize(waypoints.size());
    size_t vertexIndex { 0 };
    const auto& trees = doc["trees"].GetArray();
    for(const auto& tree: trees) {
        const auto neighbours { tree.GetArray() };
        for(const auto& neighbour: neighbours) {
            const auto action { !strcmp(neighbour["action"].GetString(), "move")? Action::move: Action::jump };
            adj[vertexIndex].emplace_back( neighbour["vert"].GetInt(), action );
        }
        vertexIndex++;
    }
    
    const auto influence = doc["influence"].GetArray();
    areas.resize(influence.Size());
    size_t pointIndex { 0 };
    for(const auto& rect: influence) {
        const auto p { rect.GetArray() };
        for(size_t i = 0; i < 4; i++) {
            areas[pointIndex][i] = p[i].GetInt();
        }
        pointIndex++;
    }
}