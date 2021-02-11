#include "LevelScene.hpp"

#include "Unit.hpp"
#include "Bot.hpp"
#include "Warrior.hpp"
#include "Slime.hpp"
#include "Archer.hpp"
#include "units/Cannon.hpp"
#include "units/BanditBoss.hpp"
#include "objects/Stalactite.hpp"
#include "BoulderPusher.hpp"
#include "Spider.hpp"
#include "Spearman.hpp"
#include "Player.hpp"

#include "Platform.hpp"
#include "Props.hpp"
#include "Traps.hpp"

#include "PhysicsHelper.hpp"
#include "UserInputHandler.hpp"
#include "Utils.hpp"
#include "SizeDeducer.hpp"
#include "Interface.hpp"
#include "TileMapParser.hpp"
#include "Path.hpp"
#include "ContactHandler.hpp"
#include "ParallaxBackground.hpp"

#include <unordered_map>

LevelScene::LevelScene(int id) : 
    m_id{ id } 
{
}

cocos2d::Scene* LevelScene::createRootScene(int id) {
    const auto root = cocos2d::Scene::createWithPhysics();
    const auto world = root->getPhysicsWorld();
    world->setGravity(cocos2d::Vec2(0, GRAVITY));
    world->setSubsteps(2);
#ifndef COCOS2D_DEBUG
    world->setFixedUpdateRate(60);
#endif 
    world->setDebugDrawMask(cocos2d::PhysicsWorld::DEBUGDRAW_NONE);

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

    m_tmxFile = cocos2d::StringUtils::format("Map/level_%d.tmx", m_id);
    	
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
    const auto tileMap = this->getChildByName<cocos2d::FastTMXTiledMap*>("Map");
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
    /*To navigate back to native iOS screen(if present) without quitting the application, 
    do not use Director::getInstance()->end() as given above,
    instead trigger a custom event created in RootViewController.mm as below*/
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void LevelScene::InitTileMapObjects(cocos2d::FastTMXTiledMap * map) {

    if(!m_parser) {
        m_parser = std::make_unique<TileMapParser>(map, m_tmxFile);
        m_parser->Parse();
    }

    /// TODO: get rid of this shitty maps
    std::unordered_map<size_t, Path> paths;
    std::unordered_map<size_t, size_t> pathIdByUnitId;
    std::unordered_map<size_t, cocos2d::Rect> influences;
    std::unordered_map<size_t, Enemies::Warrior*> warriors;
    std::unordered_map<size_t, Enemies::Slime*> slimes;
    std::unordered_map<size_t, Enemies::Archer*> archers;
    std::unordered_map<size_t, Enemies::Cannon*> cannons;
    std::unordered_map<size_t, Enemies::Stalactite*> stalactites;
    std::unordered_map<size_t, Enemies::BoulderPusher*> boulderPushers;
    std::unordered_map<size_t, Enemies::Spider*> spiders;
    Enemies::BanditBoss * boss { nullptr };
    size_t bossId { 0 };


    influences.reserve(40);
    paths.reserve(30);
    pathIdByUnitId.reserve(20);
    warriors.reserve(20);
    archers.reserve(10);
    cannons.reserve(10);
    stalactites.reserve(10);
    boulderPushers.reserve(10);
    spiders.reserve(20);

    for(size_t i = 0; i < Utils::EnumSize<core::CategoryName>(); i++) {
        const auto category { static_cast<core::CategoryName>(i) };
        const auto parsedForms { m_parser->Peek(category) };
        for(const auto& form: parsedForms) {
            if(form.m_type == core::CategoryName::PLAYER) {
                const auto contentSize = form.m_rect.size * form.m_scale;
                const auto hero { Player::create(contentSize) };
                hero->setName(core::EntityNames::PLAYER);
                hero->setPosition( form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height } );
                map->addChild(hero, 100);
            } 
            else if(form.m_type == core::CategoryName::PLATFORM) {
                const auto platform = Platform::create(form.m_rect.size);
                platform->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(platform);
            }
            else if(form.m_type == core::CategoryName::BORDER) {
                const auto border { cocos2d::Node::create() };
                ///< The density of the object.
                ///< The bounciness of the physics body.
                ///< The roughness of the surface of a shape.
                auto body = cocos2d::PhysicsBody::createEdgeChain(
                    form.m_points.data()
                    , form.m_points.size()
                    , {0.5f, 0.f, 0.1f}
                    , 2.f
                );
                body->setCategoryBitmask(Utils::CreateMask(core::CategoryBits::BOUNDARY));
                body->setCollisionBitmask(
                    Utils::CreateMask(
                        core::CategoryBits::ENEMY 
                        , core::CategoryBits::PLAYER
                        , core::CategoryBits::ENEMY_PROJECTILE
                    )
                );
                body->setContactTestBitmask(
                    Utils::CreateMask(
                        core::CategoryBits::GROUND_SENSOR
                        , core::CategoryBits::ENEMY_PROJECTILE
                        , core::CategoryBits::PLAYER_PROJECTILE
                    )
                );
                border->addComponent(body);
                map->addChild(border);
            }
            else if(form.m_type == core::CategoryName::SPIKES) {
                const auto trap = Traps::Spikes::create(form.m_rect.size);
                trap->setPosition(form.m_rect.origin + form.m_rect.size / 2.f);
                map->addChild(trap);
            }
            else if(form.m_type == core::CategoryName::PROPS) {
                const auto prop = props::Prop::create(
                        Utils::EnumCast<props::Name>(form.m_subType)
                        , form.m_rect.size
                        , form.m_scale
                );
                prop->setAnchorPoint({0.5f, 0.f});
                prop->setPosition(form.m_rect.origin + prop->getContentSize());
                map->addChild(prop);
            }
            else if(form.m_type == core::CategoryName::PATH) {
                Path path{};
                path.m_waypoints = form.m_points;
                path.m_id = form.m_id;
                paths.emplace(form.m_id, std::move(path));
            }
            else if(form.m_type == core::CategoryName::ENEMY) {
                const auto zOrder { 10 };
                const auto contentSize = form.m_rect.size * form.m_scale;
                switch(Utils::EnumCast<core::EnemyClass>(form.m_subType)) {
                    case core::EnemyClass::WARRIOR: {
                        const auto warrior { Enemies::Warrior::create(form.m_id, contentSize) };
                        warrior->setName(core::EntityNames::WARRIOR);
                        warrior->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(warrior, zOrder);
                        warriors.emplace(form.m_id, warrior);
                        pathIdByUnitId.emplace(form.m_id, form.m_pathId);
                    } break;
                    case core::EnemyClass::BOSS: {
                        bossId = form.m_id;
                        boss = Enemies::BanditBoss::create(form.m_id, contentSize);
                        boss->setName(core::EntityNames::BOSS);
                        boss->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(boss, zOrder);
                        pathIdByUnitId.emplace(form.m_id, form.m_pathId);
                    } break;
                    case core::EnemyClass::SLIME: {
                        const auto slime { Enemies::Slime::create(form.m_id, contentSize) };
                        slime->setName(core::EntityNames::SLIME);
                        slime->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(slime, zOrder);
                        slimes.emplace(form.m_id, slime);
                        pathIdByUnitId.emplace(form.m_id, form.m_pathId);
                    } break;
                    case core::EnemyClass::SPEARMAN: {
                        const auto spearman { Enemies::Spearman::create(form.m_id, contentSize) };
                        spearman->setName(core::EntityNames::SPEARMAN);
                        spearman->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(spearman, zOrder);
                        warriors.emplace(form.m_id, spearman);
                        pathIdByUnitId.emplace(form.m_id, form.m_pathId);
                    } break;
                    case core::EnemyClass::ARCHER: {
                        const auto archer { Enemies::Archer::create(form.m_id, contentSize) };
                        archer->setName(core::EntityNames::ARCHER);
                        archer->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(archer, zOrder);
                        archers.emplace(form.m_id, archer);
                    } break;
                    case core::EnemyClass::CANNON: {
                        const auto cannon { Enemies::Cannon::create(form.m_id, contentSize, form.m_scale) };
                        if(form.m_flipX) cannon->Turn();
                        cannon->setName(core::EntityNames::CANNON);
                        cannon->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(cannon, zOrder);
                        cannons.emplace(form.m_id, cannon);
                    } break;
                    case core::EnemyClass::STALACTITE: {
                        const auto stalactite { Enemies::Stalactite::create(form.m_id, contentSize, form.m_scale) };
                        stalactite->setName(core::EntityNames::STALACTITE);
                        stalactite->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(stalactite, zOrder);
                        stalactites.emplace(form.m_id, stalactite);
                    } break;
                    case core::EnemyClass::BOULDER_PUSHER: {
                        const auto boulderPusher { Enemies::BoulderPusher::create(form.m_id, contentSize) };
                        boulderPusher->setName(core::EntityNames::BOULDER_PUSHER);
                        boulderPusher->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(boulderPusher, zOrder);
                        boulderPushers.emplace(form.m_id, boulderPusher);
                    } break;
                    case core::EnemyClass::SPIDER: {
                        const auto spider { Enemies::Spider::create(form.m_id, contentSize) };
                        spider->setName(core::EntityNames::SPIDER);
                        spider->setPosition(form.m_rect.origin + cocos2d::Size{ contentSize.width / 2.f, contentSize.height });
                        map->addChild(spider, zOrder);
                        spider->CreateWebAt(spider->getPosition() + cocos2d::Vec2{0.f, spider->getContentSize().height });
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
    for(auto& [id, slime]: slimes) {
        const auto pathId { pathIdByUnitId.at(id) };
        slime->AttachNavigator(std::move(paths.at(pathId)));
        if(auto it = influences.find(id); it != influences.end()) {
            slime->AttachInfluenceArea(it->second);
        }
    } 
    for(auto& [id, spider]: spiders) {
        const auto pathId { pathIdByUnitId.at(id) };
        spider->AttachNavigator(std::move(paths.at(pathId)));
    } 
    for(auto& [id, archer]: archers) {
        archer->AttachInfluenceArea(influences.at(id));
    } 
    for(auto& [id, cannon]: cannons) {
        cannon->AttachInfluenceArea(influences.at(id));
    } 
    for(auto& [id, stalactite]: stalactites) {
        stalactite->AttachInfluenceArea(influences.at(id));
    } 
    for(auto& [id, pusher]: boulderPushers) {
        pusher->AttachInfluenceArea(influences.at(id));
    } 

    if(auto it = influences.find(bossId); it != influences.end()) {
        boss->AttachInfluenceArea(it->second);
    }
}
