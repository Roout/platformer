#include "LevelScene.h"
#include "Unit.h"
#include "PhysicWorld.h"
#include "UnitView.h"
#include "userInputHandler.h"

LevelScene::LevelScene(int id): 
    m_id{id} 
{
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
	this->scheduleUpdate();

	const auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
	const cocos2d::Vec2 origin = cocos2d::Director::getInstance()->getVisibleOrigin();
    
    const auto tmxFile { cocos2d::StringUtils::format("Map/level_%d.tmx", m_id) };
    cocos2d::FastTMXTiledMap *m_tileMap { cocos2d::FastTMXTiledMap::create(tmxFile) };
    
    // m_tileMap->setAnchorPoint( {0.5f, 0.5f} );
    // m_tileMap->setPosition( cocos2d::Vec2{visibleSize.width, visibleSize.height } / 2.f + origin / 2.f);
    // m_tileMap->setScale(2.f); // temporary

    this->addChild(m_tileMap);

    m_world = std::make_unique<PhysicWorld>(); 

    cocos2d::Vec2 playerPosition {};
    // TODO:
    // read player position from the map
    const auto group = m_tileMap->getObjectGroup("objects");
	if (group) {
		const auto allObjects = group->getObjects();
		for (const auto& object : allObjects) {
			const auto& objMap = object.asValueMap();
			const auto type = objMap.at("type").asString();
			const auto name = objMap.at("name").asString();
			// TODO: change to object names
			if (type == "point") {
				auto x = objMap.at("x").asFloat();
				auto y = objMap.at("y").asFloat();
				if (name == "player") {
					playerPosition = {x, y};
				}
			}
		}
	}
    // TODO: 
    // add some static physic bodies to test PhysicWorld class.
    const auto obstaclesLayer = m_tileMap->getLayer("ground");
	if (obstaclesLayer) {
		const auto tileSize = obstaclesLayer->getMapTileSize();
		const auto mapSize = obstaclesLayer->getLayerSize();
		const auto width{ static_cast<int>(mapSize.width) };
		const auto height{ static_cast<int>(mapSize.height) };
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				const auto tileGid = obstaclesLayer->getTileGIDAt({
					static_cast<float>(i),
					static_cast<float>(j) }
				);
				if (tileGid) {
					const auto properties = m_tileMap->getPropertiesForGID(tileGid).asValueMap();
					const bool isCollidable {
                        properties.count("collidable") > 0 && 
                        properties.at("collidable").asBool()
                    };
					if (isCollidable) {
                        auto body = m_world->Create<StaticBody>( 
                            { i * tileSize.width, (height - j - 1) * tileSize.height }, 
                            { tileSize.width, tileSize.height }
                        );
                        body->SetMask(
                            CreateMask(CategoryBits::BOUNDARY),
                            CreateMask(CategoryBits::ENEMY,CategoryBits::HERO) 
                        );
					}

				}
			}
		}
	}

    m_unit = std::make_unique<Unit>(m_world.get(), playerPosition.x, playerPosition.y);
    m_inputHandler = std::make_unique<UserInputHandler>(m_unit.get(), this);

    auto playerNode { HeroView::create(m_unit.get())};
    m_tileMap->addChild(playerNode, 10);
    auto followTheHero = cocos2d::Follow::create(playerNode);
	m_tileMap->runAction(followTheHero);

    return true;
}

void LevelScene::menuCloseCallback(cocos2d::Ref* pSender) {
    //Close the cocos2d-x game scene and quit the application
    cocos2d::Director::getInstance()->end();
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void LevelScene::update(float dt) {
    m_world->Step(dt, 1);
}
