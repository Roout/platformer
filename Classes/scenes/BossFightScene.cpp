#include "BossFightScene.hpp"
#include "Interface.hpp"

#include "components/ParallaxBackground.hpp"
#include "components/Movement.hpp"

BossFightScene::BossFightScene(int id) 
    : LevelScene {id}
{}

cocos2d::Scene* BossFightScene::createRootScene(int id) {
    const auto root = cocos2d::Scene::createWithPhysics();
    const auto world = root->getPhysicsWorld();
    world->setGravity(cocos2d::Vec2(0, LevelScene::GRAVITY));
    world->setSubsteps(2);
#ifndef COCOS2D_DEBUG
    world->setFixedUpdateRate(60);
#endif 
    world->setDebugDrawMask(cocos2d::PhysicsWorld::DEBUGDRAW_NONE);

    const auto uInterface = Interface::create();
    const auto level = BossFightScene::create(id);
    
    root->addChild(level);
    root->addChild(uInterface, level->getLocalZOrder() + 1);
    
    return root;
}

BossFightScene* BossFightScene::create(int id) {
    auto *pRet = new(std::nothrow) BossFightScene{id};
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
} 

bool BossFightScene::init() {
    if (!cocos2d::Scene::init()) {
		return false;
	}
    
    this->setName("Level");
	this->scheduleUpdate();

    m_tmxFile = cocos2d::StringUtils::format("Map/level_%d_boss.tmx", m_id);
    	
    const auto tileMap { cocos2d::FastTMXTiledMap::create(m_tmxFile) };
    tileMap->setName("Map");
    this->addChild(tileMap);
    
    // add parallax background
    auto back = Background::create(tileMap->getContentSize());
    tileMap->addChild(back, -1);

    // mark untouchable layers:
    for(auto& child: tileMap->getChildren()) {
        child->setName("Untouchable");
    }

    this->Restart();    
    
    back->setAnchorPoint({0.f, 0.f});
    back->setPosition(-tileMap->getPosition());

    return true;
}
