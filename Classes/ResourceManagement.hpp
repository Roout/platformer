#ifndef RESOURCE_MANAGEMENT_HPP
#define RESOURCE_MANAGEMENT_HPP

#include "dragonBones/DragonBonesHeaders.h"
#include "dragonBones/cocos2dx/CCDragonBonesHeaders.h"

#include <string>

namespace Resource {

    inline dragonBones::CCArmatureDisplay* BuildArmatureDisplay(
        const std::string& cacheName
    ) {
        const auto factory = dragonBones::CCFactory::getFactory();
        if(const auto bonesData = factory->getDragonBonesData(cacheName); bonesData == nullptr) {
            factory->loadDragonBonesData(cacheName + "/" + cacheName + "_ske.json");
        }
        if(const auto texture = factory->getTextureAtlasData(cacheName); texture == nullptr) {
            factory->loadTextureAtlasData(cacheName + "/" + cacheName + "_tex.json");
        }
        return factory->buildArmatureDisplay("Armature", cacheName);
    }
}

#endif // RESOURCE_MANAGEMENT_HPP