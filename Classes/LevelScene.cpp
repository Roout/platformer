#include "LevelScene.hpp"
#include "Unit.hpp"
#include "PhysicWorld.hpp"
#include "UnitView.hpp"
#include "UserInputHandler.hpp"
#include "TileMapParser.hpp"

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
    cocos2d::FastTMXTiledMap *tileMap { cocos2d::FastTMXTiledMap::create(tmxFile) };
    
    // tileMap->setAnchorPoint( {0.5f, 0.5f} );
    // tileMap->setPosition( cocos2d::Vec2{visibleSize.width, visibleSize.height } / 2.f + origin / 2.f);
    // tileMap->setScale(2.f); // temporary

    this->addChild(tileMap);

    m_world = std::make_unique<PhysicWorld>(); 

    TileMapParser parser{ tileMap };
    parser.Parse();

    const auto playerPosition { parser.Acquire<ParsedType::HERO_POSITION>() };
    const auto obstacles { parser.Acquire<ParsedType::STATIC_BODIES>()}; 

    for(const auto& shape : obstacles ) {
        auto body = m_world->Create<StaticBody>(shape.origin, shape.size);
        body->SetMask(
            CreateMask(CategoryBits::BOUNDARY),
            CreateMask(CategoryBits::ENEMY,CategoryBits::HERO) 
        );
    }

    m_unit = std::make_unique<Unit>(m_world.get(), playerPosition.x, playerPosition.y);
    m_inputHandler = std::make_unique<UserInputHandler>(m_unit.get(), this);

    auto playerNode { HeroView::create(m_unit.get())};
    tileMap->addChild(playerNode, 10);
    auto followTheHero = cocos2d::Follow::create(playerNode);
	tileMap->runAction(followTheHero);

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
