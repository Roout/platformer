#include "EnemyPath.hpp"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"

#include "cocos2d.h"

#include <string>

rapidjson::Document path::Forest::Load(size_t id) {
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

void path::Forest::Parse(rapidjson::Document& doc) {
    waypoints.reserve(100);
    const auto& points = doc["waypoints"].GetArray(); 
    for(const auto& p: points) {
        waypoints.emplace_back(p[0].GetInt(), p[1].GetInt());
    }

    adj.resize(waypoints.size());
    size_t vertexIndex { 0 };
    const auto& trees = doc["trees"].GetArray();
    for(const auto& tree: trees) {
        const auto& neighbours { tree.GetArray() };
        for(const auto& neighbour: neighbours) {
            adj[vertexIndex].emplace_back(
                neighbour["vert"].GetInt(), 
                (neighbour["action"].GetString() == "move"? Action::move: Action::jump)
            );
        }
        vertexIndex++;
    }
}