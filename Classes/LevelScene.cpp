#include "LevelScene.hpp"
#include "Unit.hpp"
#include "PhysicWorld.hpp"
#include "UnitView.hpp"
#include "UserInputHandler.hpp"
#include "TileMapParser.hpp"
#include "BarrelManager.hpp"
#include "SmoothFollower.hpp"

LevelScene::LevelScene(int id): 
    m_id{ id } 
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
    tileMap->setName("Map");
    // tileMap->setAnchorPoint( {0.5f, 0.5f} );
    // tileMap->setPosition( cocos2d::Vec2{visibleSize.width, visibleSize.height } / 2.f + origin / 2.f);
    // tileMap->setScale(2.f); // temporary

    this->addChild(tileMap);

    m_world = std::make_unique<PhysicWorld>(); 

    TileMapParser parser{ tileMap };
    parser.Parse();

    const auto playerPosition { parser.Acquire<ParsedType::PLAYER>() };
    const auto obstacles { parser.Acquire<ParsedType::STATIC_BODIES>()}; 

    m_borders.reserve(5000);
    m_platforms.reserve(200);

    m_barrelManager = std::make_unique<BarrelManager>();

    for(const auto& [shape, category] : obstacles ) {
        if (category == core::CategoryName::PLATFORM ) {
            m_platforms.emplace_back(m_world.get(), 
                shape.origin.x, shape.origin.y, 
                shape.size.width, shape.size.height 
            );
        } 
        else if(category == core::CategoryName::BORDER) {
            m_borders.emplace_back(m_world.get(), 
                shape.origin.x, shape.origin.y, 
                shape.size.width, shape.size.height 
            );
        }
        else if(category == core::CategoryName::BARREL) {
            /// TODO: move width and height either to Barrel model either to map as object info.
            static constexpr float width { 80 }, height { 100 };
            
            auto barrel { std::make_unique<Barrel>(m_world.get(), shape.origin.x, shape.origin.y, width, height ) };
            auto barrelView = BarrelView::create(barrel.get());
            
            m_barrelManager->Add(move(barrel), barrelView);

            tileMap->addChild(barrelView);
        }
    }

    m_unit              = std::make_shared<Unit>(m_world.get(), playerPosition.x, playerPosition.y);
    m_playerFollower    = std::make_unique<SmoothFollower>(m_unit);
    m_inputHandler      = std::make_unique<UserInputHandler>(m_unit.get(), this);

    auto playerNode { HeroView::create(m_unit.get()) };
    tileMap->addChild(playerNode, 10);

    ///TODO: set map position so that the player will be visible at the center
    const auto shift { playerPosition 
        - cocos2d::Vec2{ visibleSize.width / 2.f, visibleSize.height / 3.f } 
        - origin / 2.f 
    };
    tileMap->setPosition(-shift);


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
   
    m_unit->UpdateWeapon(dt);
    m_world->Step(dt, 1);
    m_unit->UpdateState(dt);
    
    
    m_playerFollower->UpdateAfterUnitMove(dt);
    auto director { cocos2d::Director::getInstance() };
    const auto visibleSize = director->getVisibleSize();
	const auto origin = director->getVisibleOrigin();
    auto mapNode = this->getChildByName("Map");
    m_playerFollower->UpdateNodePosition(mapNode);

    m_barrelManager->Update();

    static unsigned int x { 0 };
    cocos2d::log("Update: %0.4f, %d", dt, x++);
    cocos2d::log("Unit is at: [%0.4f, %0.4f]", 
        m_unit->GetBody()->GetShape().origin.x, 
        m_unit->GetBody()->GetShape().origin.y 
    );
}
