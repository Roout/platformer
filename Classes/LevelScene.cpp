#include "LevelScene.hpp"

#include "Unit.hpp"
#include "Bot.hpp"
#include "Warrior.hpp"
#include "Archer.hpp"
#include "Spider.hpp"
#include "Spearman.hpp"
#include "Player.hpp"

#include "Platform.hpp"
#include "Barrel.hpp"
#include "Border.hpp"
#include "Traps.hpp"

#include "PhysicsHelper.hpp"
#include "UserInputHandler.hpp"
#include "Utils.hpp"
#include "SizeDeducer.hpp"
#include "Interface.hpp"
#include "TileMapParser.hpp"
#include "Path.hpp"
#include "ContactHandler.hpp"

#include <unordered_map>

LevelScene::LevelScene(int id): 
    m_id{ id } 
{
}

cocos2d::Scene* LevelScene::createRootScene(int id) {
    const auto root = cocos2d::Scene::createWithPhysics();
    const auto world = root->getPhysicsWorld();
    world->setGravity(cocos2d::Vec2(0, -1000));
    world->setSubsteps(2);
    world->setDebugDrawMask(cocos2d::PhysicsWorld::DEBUGDRAW_ALL);

    const auto uInterface = Interface::create();
    const auto level = LevelScene::create(id);
    
    root->addChild(level);
    root->addChild(uInterface, level->getLocalZOrder() + 1);
    
    return root;
}

LevelScene* LevelScene::create(int id) {
    auto *pRet = new(std::nothrow) LevelScene{id};
    if (pRet && pRet->init()) {
        pRet->autorelease();
    }
    else {
        delete pRet;
        pRet = nullptr;
    }
    return pRet;
} 

bool LevelScene::init() {
	if (!cocos2d::Scene::init()) {
		return false;
	}
    
    this->setName("Level");
	this->scheduleUpdate();
    
    const auto tmxFile { cocos2d::StringUtils::format("Map/level_%d.tmx", m_id) };
    const auto tileMap { cocos2d::FastTMXTiledMap::create(tmxFile) };
    tileMap->setName("Map");
    this->addChild(tileMap);

    // mark untouchable layers:
    for(auto& child: tileMap->getChildren()) {
        child->setName("Untouchable");
    }

    this->Restart();    

    return true;
}

void LevelScene::onEnter() {
    cocos2d::Node::onEnter();
    // Add physics body contact listener
    const auto shapeContactListener = cocos2d::EventListenerPhysicsContact::create();
    shapeContactListener->onContactBegin = contact::OnContactBegin;
    shapeContactListener->onContactSeparate = contact::OnContactSeparate;
    this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(shapeContactListener, this);
}

void LevelScene::onExit() {
    cocos2d::Node::onExit();
    this->getEventDispatcher()->removeAllEventListeners();
}

void LevelScene::pause() {
    // pause for this target
    // - event system
    // - scheduler
    // - actions
    cocos2d::Node::pause();
    const auto map = this->getChildByName("Map");
    map->pause();
    for(auto& child: map->getChildren()) {
        child->pause();
    }
    // pause physic world
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    runningScene->getPhysicsWorld()->setSpeed(0.f);
}

void LevelScene::resume() {
    // resume for this target
    // - event system
    // - scheduler
    // - actions
    cocos2d::Node::resume();
    // resume all children of the tilemap
    const auto map = this->getChildByName("Map");
    for(const auto child: map->getChildren()) {
        child->resume();
    }
    map->resume();
    // resume physic world
    const auto runningScene { cocos2d::Director::getInstance()->getRunningScene() };
    runningScene->getPhysicsWorld()->setSpeed(1.0);
};

void LevelScene::Restart() {
    // Tilemap:
    // - remove children exсept layers and objects.
    const auto tileMap = dynamic_cast<cocos2d::FastTMXTiledMap*>(this->getChildByName("Map"));
    std::vector<cocos2d::Node*> scheduledForRemove;
    scheduledForRemove.reserve(100);
    for(auto& child: tileMap->getChildren()) {
        if(child->getName() != "Untouchable") {
            scheduledForRemove.emplace_back(child);
        }
    }
    for(auto & child : scheduledForRemove) {
        child->removeFromParent();
    }
    this->InitTileMapObjects(tileMap);
   
    // - reset position
    const auto player { tileMap->getChildByName(core::EntityNames::PLAYER) };

    const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
	const auto origin = cocos2d::Director::getInstance()->getVisibleOrigin();
    const auto mapShift { player->getPosition() 
        - cocos2d::Vec2{ visibleSize.width / 2.f, visibleSize.height / 3.f } 
        - origin 
    };
    tileMap->setPosition(-mapShift);
}


void LevelScene::menuCloseCallback(cocos2d::Ref* pSender) {
    //Close the cocos2d-x game scene and quit the application
    cocos2d::Director::getInstance()->end();
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void LevelScene::InitTileMapObjects(cocos2d::FastTMXTiledMap * map) {

    if(!m_parser) {
        m_parser = std::make_unique<TileMapParser>(map);
        m_parser->Parse();
    }

    /// TODO: get rid of this shitty maps
    std::unordered_map<size_t, Path> paths;
    std::unordered_map<size_t, size_t> pathIdByUnitId;
    std::unordered_map<size_t, cocos2d::Rect> influences;
    std::unordered_map<size_t, Enemies::Warrior*> warriors;
    std::unordered_map<size_t, Enemies::Archer*> archers;
    std::unordered_map<size_t, Enemies::Spider*> spiders;

    influences.reserve(100);
    paths.reserve(100);
    pathIdByUnitId.reserve(100);
    warriors.reserve(100);
    archers.reserve(100);
    spiders.reserve(100);

    for(size_t i = 0; i < Utils::EnumSize<core::CategoryName>(); i++) {
        const auto category { static_cast<core::CategoryName>(i) };
        const auto parsedForms { m_parser->Peek(category) };
        for(const auto& form: parsedForms) {
            if(form.m_type == core::CategoryName::PLAYER) {
                const auto hero { Player::create() };
                hero->setName(core::EntityNames::PLAYER);
                hero->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
                hero->setPosition(form.m_points.front());
                map->addChild(hero, 100);
            } 
            else if(form.m_type == core::CategoryName::PLATFORM) {
                const auto platform = Platform::create(form.m_rect.size);
                platform->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(platform);
            }
            else if(form.m_type == core::CategoryName::BORDER) {
                const auto border { Border::create(form.m_rect.size) };
                border->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(border);
            }
            else if(form.m_type == core::CategoryName::SPIKES) {
                const auto trap = Traps::Spikes::create(form.m_rect.size);
                trap->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(trap);
            }
            else if(form.m_type == core::CategoryName::BARREL) {
                const auto barrel { Barrel::create() };
                barrel->setPosition(form.m_points.front());
                barrel->setAnchorPoint(cocos2d::Vec2::ANCHOR_BOTTOM_LEFT);
                map->addChild(barrel);
            }
            else if(form.m_type == core::CategoryName::PATH) {
                Path path{};
                path.m_waypoints = form.m_points;
                path.m_id = form.m_id;
                paths.emplace(form.m_id, std::move(path));
            }
            else if(form.m_type == core::CategoryName::ENEMY) {
                const auto zOrder { 10 };
                switch(form.m_enemyClass) {
                    case core::EnemyClass::WARRIOR: {
                        const auto warrior { Enemies::Warrior::create(form.m_id) };
                        warrior->setName(core::EntityNames::WARRIOR);
                        warrior->setPosition(form.m_points.front());
                        map->addChild(warrior, zOrder);
                        // save warrior pointer
                        warriors.emplace(form.m_id, warrior);
                        pathIdByUnitId.emplace(form.m_id, form.m_pathId);
                    } break;
                    case core::EnemyClass::SPEARMAN: {
                        const auto spearman { Enemies::Spearman::create(form.m_id) };
                        spearman->setName(core::EntityNames::SPEARMAN);
                        spearman->setPosition(form.m_points.front());
                        map->addChild(spearman, zOrder);
                        // save spearman pointer
                        warriors.emplace(form.m_id, spearman);
                        pathIdByUnitId.emplace(form.m_id, form.m_pathId);
                    } break;
                    case core::EnemyClass::ARCHER: {
                        const auto archer { Enemies::Archer::create(form.m_id) };
                        archer->setName(core::EntityNames::ARCHER);
                        archer->setPosition(form.m_points.front());
                        map->addChild(archer, zOrder);
                        // save warrior pointer
                        archers.emplace(form.m_id, archer);
                    } break;
                    case core::EnemyClass::SPIDER: {
                        const auto spider { Enemies::Spider::create(form.m_id) };
                        spider->setName(core::EntityNames::SPIDER);
                        spider->setPosition(form.m_points.front());
                        map->addChild(spider, zOrder);
                        // save warrior pointer
                        spiders.emplace(form.m_id, spider);
                        pathIdByUnitId.emplace(form.m_id, form.m_pathId);
                    } break;
                    default: break;
                }
            }
            else if(form.m_type == core::CategoryName::INFLUENCE) {
                // save component data
                influences.emplace(form.m_ownerId, form.m_rect);
            }
        }
    }
    // attach influence to warriors
    for(auto& [id, warrior]: warriors) {
        const auto pathId { pathIdByUnitId.at(id) };
        warrior->AttachNavigator(std::move(paths.at(pathId)));
        if(auto it = influences.find(id); it != influences.end()) {
            warrior->AttachInfluenceArea(it->second);
        }
    } 
    for(auto& [id, spider]: spiders) {
        const auto pathId { pathIdByUnitId.at(id) };
        spider->AttachNavigator(std::move(paths.at(pathId)));
    } 
    for(auto& [id, archer]: archers) {
        archer->AttachInfluenceArea(influences.at(id));
    } 
}
